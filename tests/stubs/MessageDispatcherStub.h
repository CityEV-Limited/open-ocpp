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

#ifndef MESSAGEDISPATCHERSTUB_H
#define MESSAGEDISPATCHERSTUB_H

#include "IMessageDispatcher.h"

#include <map>
#include <string>

namespace ocpp
{
namespace messages
{

/** @brief Messages dispatcher stub for unit tests */
class MessageDispatcherStub : public IMessageDispatcher
{
  public:
    /** @brief Constructor */
    MessageDispatcherStub();
    /** @brief Destructor */
    virtual ~MessageDispatcherStub();

    /** @copydoc bool IMessageDispatcher::registerHandler(const std::string&, IMessageHandler&) */
    bool registerHandler(const std::string& action, IMessageHandler& handler) override;

    /** @copydoc bool IMessageDispatcher::dispatchMessage(const std::string&,
                                                          const rapidjson::Value&,
                                                          rapidjson::Document&,
                                                          const char*&,
                                                          std::string&) */
    bool dispatchMessage(const std::string&      action,
                         const rapidjson::Value& payload,
                         rapidjson::Document&    response,
                         const char*&            error_code,
                         std::string&            error_message) override;

    /** @brief Check if a specific action as a registered handler */
    bool hasHandler(const std::string& action) const;

  private:
    /** @brief Handlers */
    std::map<std::string, IMessageHandler*> m_handlers;
};

} // namespace messages
} // namespace ocpp

#endif // MESSAGEDISPATCHERSTUB_H
