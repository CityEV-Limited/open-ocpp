/*
Copyright (c) 2020 Cedric Jimenez
This file is part of OpenOCPP.

OpenOCPP is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenOCPP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenOCPP. If not, see <http://www.gnu.org/licenses/>.
*/

#include "RpcBase.h"

#include <functional>
#include <sstream>

namespace ocpp
{
namespace rpc
{

/** @brief RPC call type */
static constexpr const char* CALL = "2";
/** @brief RPC call result type */
static constexpr const char* CALLRESULT = "3";
/** @brief RPC call error type */
static constexpr const char* CALLERROR = "4";

/** @brief Constructor */
RpcBase::RpcBase()
    : m_rpc_listener(nullptr), m_spies(), m_transaction_id(0), m_call_mutex(), m_requests_queue(), m_results_queue(), m_rx_thread(nullptr)
{
}

/** @brief Destructor */
RpcBase::~RpcBase()
{
    stop();
}

/** @copydoc bool IRpc::call(const std::string&, const rapidjson::Document&, rapidjson::Document&, std::chrono::milliseconds) */
bool RpcBase::call(const std::string&         action,
                   const rapidjson::Document& payload,
                   rapidjson::Document&       response,
                   std::chrono::milliseconds  timeout)
{
    bool ret = false;

    // Check connection state
    if (isConnected())
    {
        // Only one RPC request/response at a time
        std::lock_guard<std::mutex> call_lock(m_call_mutex);

        // Serialize message
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        payload.Accept(writer);

        std::stringstream serialized_message;
        serialized_message << "[";
        serialized_message << CALL << ", ";
        serialized_message << "\"" << m_transaction_id << "\", ";
        serialized_message << "\"" << action << "\", ";
        serialized_message << buffer.GetString();
        serialized_message << "]";

        // Initialize call context
        std::stringstream id;
        id << m_transaction_id;

        // Send message
        std::string msg = serialized_message.str();
        if (send(msg))
        {
            // Wait for response
            std::stringstream expected_id;
            expected_id << m_transaction_id;
            RpcMessage* rpc_message = nullptr;
            auto        wait_time   = std::chrono::steady_clock().now() + timeout;
            do
            {
                // Compute timeout
                auto now       = std::chrono::steady_clock().now();
                auto left_time = std::chrono::duration_cast<std::chrono::milliseconds>(wait_time - now);
                if (left_time.count() >= 0)
                {
                    // Wait for a message
                    ret = m_results_queue.pop(rpc_message, left_time.count());
                    if (ret)
                    {
                        // Check id
                        if (rpc_message->unique_id != expected_id.str())
                        {
                            // Wrong message
                            delete rpc_message;
                            rpc_message = nullptr;
                        }
                    }
                }
            } while (ret && (rpc_message == nullptr));

            // Extract response
            if (rpc_message)
            {
                response.CopyFrom(rpc_message->payload, response.GetAllocator());
                delete rpc_message;
            }
            else
            {
                ret = false;
            }
        }

        // Update transaction id
        m_transaction_id++;
    }

    return ret;
}

/** @copydoc void IRpc::registerListener(IListener&) */
void RpcBase::registerListener(IRpc::IListener& listener)
{
    m_rpc_listener = &listener;
}

/** @copydoc void IRpc::registerSpy(ISpy&) */
void RpcBase::registerSpy(IRpc::ISpy& spy)
{
    m_spies.push_back(&spy);
}

// RpcBase interface

/** @brief Start RPC operations */
void RpcBase::start()
{
    // Check if already started
    if (!m_rx_thread)
    {
        // Initialize transaction id sequence
        m_transaction_id = std::rand();

        // Flush queues
        m_requests_queue.clear();
        m_requests_queue.setEnable(true);
        m_results_queue.clear();

        // Start reception thread
        m_rx_thread = new std::thread(std::bind(&RpcBase::rxThread, this));
    }
}

/** @brief Stop RPC operations */
void RpcBase::stop()
{
    // Check if already started
    if (m_rx_thread)
    {
        // Stop reception thread
        m_requests_queue.setEnable(false);
        m_rx_thread->join();
        delete m_rx_thread;
        m_rx_thread = nullptr;
    }
}

/** @brief Process received data */
void RpcBase::processReceivedData(const void* data, size_t size)
{
    // Decode received data
    std::string received_data(reinterpret_cast<const char*>(data), size);
    for (ISpy* spy : m_spies)
    {
        spy->rcpMessageReceived(received_data);
    }

    // RPC frame must be a JSON array
    bool                valid = false;
    rapidjson::Document rpc_frame;
    try
    {
        rpc_frame.Parse(received_data.c_str());
        valid = !rpc_frame.HasParseError();
    }
    catch (const std::exception&)
    {
    }
    if (valid && rpc_frame.IsArray() && (rpc_frame.Size() >= 3))
    {
        // Extract message type
        const rapidjson::Value& msg_type_value = rpc_frame[0];
        if (msg_type_value.IsUint())
        {
            // Check message type
            MessageType  msg_type     = MessageType::INVALID;
            unsigned int msg_type_int = msg_type_value.GetUint();
            switch (msg_type_int)
            {
                case static_cast<unsigned int>(MessageType::CALL):
                    msg_type = MessageType::CALL;
                    valid    = (rpc_frame.Size() == 4u);
                    break;
                case static_cast<unsigned int>(MessageType::CALLRESULT):
                    msg_type = MessageType::CALLRESULT;
                    valid    = (rpc_frame.Size() == 3u);
                    break;
                case static_cast<unsigned int>(MessageType::CALLERROR):
                    msg_type = MessageType::CALLERROR;
                    valid    = (rpc_frame.Size() == 5u);
                    break;
                default:
                    // Unknown type
                    valid = false;
                    break;
            }
            if (valid)
            {
                // Extract unique identifier
                const rapidjson::Value& unique_id_value = rpc_frame[1];
                if (unique_id_value.IsString())
                {
                    // Decode message
                    std::string unique_id = unique_id_value.GetString();
                    switch (msg_type)
                    {
                        case MessageType::CALL:
                            valid = decodeCall(unique_id, rpc_frame[2], rpc_frame[3]);
                            break;
                        case MessageType::CALLRESULT:
                            valid = decodeCallResult(unique_id, rpc_frame[2]);
                            break;
                        case MessageType::CALLERROR:
                        default:
                            valid = decodeCallError(unique_id, rpc_frame[2], rpc_frame[3], rpc_frame[4]);
                            break;
                    }
                    if (!valid)
                    {
                        sendCallError("", RPC_ERROR_PROTOCOL, "");
                    }
                }
                else
                {
                    sendCallError("", RPC_ERROR_PROTOCOL, "");
                }
            }
            else
            {
                sendCallError("", RPC_ERROR_PROTOCOL, "");
            }
        }
        else
        {
            sendCallError("", RPC_ERROR_PROTOCOL, "");
        }
    }
    else
    {
        sendCallError("", RPC_ERROR_PROTOCOL, "");
    }
}

/** @brief Send a message throug the websocket connection */
bool RpcBase::send(const std::string& msg)
{
    // Notify spy
    for (ISpy* spy : m_spies)
    {
        spy->rcpMessageSent(msg);
    }

    // Send message
    return doSend(msg);
}

/** @brief Decode a CALL message */
bool RpcBase::decodeCall(const std::string& unique_id, const rapidjson::Value& action, const rapidjson::Value& payload)
{
    bool ret = false;

    // Check types
    if (action.IsString() && payload.IsObject())
    {
        // Add request to the queue
        RpcMessage* msg = new RpcMessage(unique_id, action.GetString(), payload);
        m_requests_queue.push(msg);

        ret = true;
    }

    return ret;
}

/** @brief Decode a CALLRESULT message */
bool RpcBase::decodeCallResult(const std::string& unique_id, const rapidjson::Value& payload)
{
    bool ret = false;

    // Check types
    if (payload.IsObject())
    {
        // Add result to the queue
        RpcMessage* msg = new RpcMessage(unique_id, payload);
        m_results_queue.push(msg);

        ret = true;
    }

    return ret;
}

/** @brief Decode a CALLERROR message */
bool RpcBase::decodeCallError(const std::string&      unique_id,
                              const rapidjson::Value& error,
                              const rapidjson::Value& message,
                              const rapidjson::Value& payload)
{
    bool ret = false;

    // Check types
    if (error.IsString() && message.IsString() && payload.IsObject())
    {
        (void)unique_id;
        ret = true;
    }

    return ret;
}

/** @brief Send a CALLERROR message */
void RpcBase::sendCallError(const std::string& unique_id, const char* error, const std::string& message)
{
    // Serialize message
    std::stringstream serialized_message;
    serialized_message << "[";
    serialized_message << CALLERROR << ", ";
    serialized_message << "\"" << unique_id << "\", ";
    serialized_message << "\"" << error << "\", ";
    serialized_message << "\"" << message << "\", ";
    serialized_message << "{}";
    serialized_message << "]";

    // Send message
    std::string msg = serialized_message.str();
    send(msg);
}

/** @brief Reception thread */
void RpcBase::rxThread()
{
    // Thread loop
    RpcMessage* rpc_message = nullptr;
    while (m_requests_queue.pop(rpc_message))
    {
        // Notify call
        rapidjson::Document response;
        std::string         error;
        const char*         error_code = nullptr;
        response.Parse("{}");
        if (m_rpc_listener->rpcCallReceived(rpc_message->action, rpc_message->payload, response, error_code, error))
        {
            // Serialize message
            rapidjson::StringBuffer                    buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            response.Accept(writer);

            std::stringstream serialized_message;
            serialized_message << "[";
            serialized_message << CALLRESULT << ", ";
            serialized_message << "\"" << rpc_message->unique_id << "\", ";
            serialized_message << buffer.GetString();
            serialized_message << "]";

            // Send message
            std::string msg = serialized_message.str();
            send(msg);
        }
        else
        {
            // Error
            if (!error_code)
            {
                error_code = RPC_ERROR_GENERIC;
            }
            sendCallError(rpc_message->unique_id, error_code, error);
        }

        // Free resources
        delete rpc_message;
    }
}

} // namespace rpc
} // namespace ocpp
