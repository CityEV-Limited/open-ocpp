/*
Copyright (c) 2020 Cedric Jimenez
This file is part of OpenOCPP.

OpenOCPP is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

OpenOCPP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenOCPP. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPENOCPP_IDTOKENINFOTYPE_H
#define OPENOCPP_IDTOKENINFOTYPE_H

#include "DateTime.h"
#include "Enums.h"
#include "Optional.h"

namespace ocpp
{
namespace types
{

/** @brief Contains status information about an identifier */
struct IdTokenInfoType
{
    /** @brief Required. Current status of the ID Token */
    AuthorizationStatus status;
    /** @brief Optional. Date and Time after which the token must be considered invalid */
    Optional<DateTime> cacheExpiryDateTime;
};

} // namespace types
} // namespace ocpp

#endif // OPENOCPP_IDTOKENINFOTYPE_H
