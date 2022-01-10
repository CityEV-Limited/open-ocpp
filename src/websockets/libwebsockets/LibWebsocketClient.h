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

#ifndef LIBWEBSOCKETCLIENT_H
#define LIBWEBSOCKETCLIENT_H

#include "IWebsocketClient.h"
#include "Queue.h"
#include "Url.h"
#include "libwebsockets.h"
#include <condition_variable>
#include <mutex>
#include <thread>

namespace ocpp
{
namespace websockets
{

/** @brief Websocket client implementation using libwebsockets */
class LibWebsocketClient : public IWebsocketClient
{
  public:
    /** @brief Constructor */
    LibWebsocketClient();
    /** @brief Destructor */
    virtual ~LibWebsocketClient();

    /** @copydoc bool IWebsocketClient::connect(const std::string&, const std::string&, const Credentials&, unsigned int, unsigned int) */
    bool connect(const std::string& url,
                 const std::string& protocol,
                 const Credentials& credentials,
                 unsigned int       connect_timeout = 5000u,
                 unsigned int       retry_interval  = 5000u) override;

    /** @copydoc bool IWebsocketClient::disconnect() */
    bool disconnect() override;

    /** @copydoc bool IWebsocketClient::isConnected() */
    bool isConnected() override;

    /** @copydoc bool IWebsocketClient::send(const void*, size_t) */
    bool send(const void* data, size_t size) override;

    /** @copydoc void IWebsocketClient::registerListener(IWebsocketClientListener&) */
    void registerListener(IWebsocketClientListener& listener) override;

  private:
    /** @brief Message to send */
    struct SendMsg
    {
        /** @brief Constructor */
        SendMsg(const void* _data, size_t _size)
        {
            data    = new unsigned char[LWS_PRE + _size];
            size    = _size;
            payload = &data[LWS_PRE];
            memcpy(payload, _data, size);
        }
        /** @brief Destructor */
        virtual ~SendMsg() { delete[] data; }

        /** @brief Data buffer */
        unsigned char* data;
        /** @brief Payload start */
        unsigned char* payload;
        /** @brief Size in bytes */
        size_t size;
    };

    /** @brief Listener */
    IWebsocketClientListener* m_listener;
    /** @brief Internal thread */
    std::thread* m_thread;
    /** @brief Indicate the end of processing to the thread */
    bool m_end;
    /** @brief Retry interval in ms */
    uint32_t m_retry_interval;
    /** @brief Indicate if the connection error has been notified at least once */
    bool m_connection_error_notified;
    /** @brief Connection URL */
    Url m_url;
    /** @brief Name of the protocol to use */
    std::string m_protocol;
    /** @brief Credentials */
    Credentials m_credentials;
    /** @brief Indicate the connection state */
    bool m_connected;

    /** @brief Websocket context */
    struct lws_context* m_context;
    /** @brief Schedule list */
    lws_sorted_usec_list_t m_sched_list;
    /** @brief Related wsi */
    struct lws* m_wsi;
    /** @brief Retry policy */
    lws_retry_bo_t m_retry_policy;
    /** @brief Consecutive retries */
    uint16_t m_retry_count;

    /** @brief Queue of messages to send */
    ocpp::helpers::Queue<SendMsg*> m_send_msgs;

    /** @brief Internal thread */
    void process();

    /** @brief libwebsockets connection callback */
    static void connectCallback(struct lws_sorted_usec_list* sul);
    /** @brief libwebsockets event callback */
    static int eventCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);
};

} // namespace websockets
} // namespace ocpp

#endif // LIBWEBSOCKETCLIENT_H
