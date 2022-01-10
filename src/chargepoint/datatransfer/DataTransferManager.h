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

#ifndef DATATRANSFERMANAGER_H
#define DATATRANSFERMANAGER_H

#include "DataTransfer.h"
#include "Enums.h"
#include "GenericMessageHandler.h"

namespace ocpp
{
// Forward declarations
namespace messages
{
class IMessageDispatcher;
class GenericMessagesConverter;
class GenericMessageSender;
} // namespace messages

// Main namespace
namespace chargepoint
{

class IChargePointEventsHandler;

/** @brief Handle charge point data transfer requests */
class DataTransferManager : public ocpp::messages::GenericMessageHandler<ocpp::messages::DataTransferReq, ocpp::messages::DataTransferConf>
{
  public:
    /** @brief Constructor */
    DataTransferManager(IChargePointEventsHandler&                      events_handler,
                        const ocpp::messages::GenericMessagesConverter& messages_converter,
                        ocpp::messages::IMessageDispatcher&             msg_dispatcher,
                        ocpp::messages::GenericMessageSender&           msg_sender);

    /** @brief Destructor */
    virtual ~DataTransferManager();

    /**
     * @brief Send a data transfer request
     * @param vendor_id Identifies the vendor specific implementation
     * @param message_id Identifies the message
     * @param request_data Data associated to the request
     * @param status Response status
     * @param response_data Data associated with the response
     * @return true if the data transfer has been done, false otherwise
     */
    bool dataTransfer(const std::string&               vendor_id,
                      const std::string&               message_id,
                      const std::string&               request_data,
                      ocpp::types::DataTransferStatus& status,
                      std::string&                     response_data);

    // GenericMessageHandler interface

    /** @copydoc bool GenericMessageHandler<RequestType, ResponseType>::handleMessage(const RequestType& request,
     *                                                                                ResponseType& response,
     *                                                                                const char*& error_code,
     *                                                                                std::string& error_message)
     */
    bool handleMessage(const ocpp::messages::DataTransferReq& request,
                       ocpp::messages::DataTransferConf&      response,
                       const char*&                           error_code,
                       std::string&                           error_message) override;

  private:
    /** @brief User defined events handler */
    IChargePointEventsHandler& m_events_handler;
    /** @brief Message sender */
    ocpp::messages::GenericMessageSender& m_msg_sender;
};

} // namespace chargepoint
} // namespace ocpp

#endif // DATATRANSFERMANAGER_H
