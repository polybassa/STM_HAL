/* Copyright (C) 2015  Nils Weiss
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "AT_Parser.h"
#include "trace.h"
#include <vector>
#include <iostream>

using app::AT;
using app::ATCmd;
using app::ATCmdOK_ERROR;
using app::ATParser;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

bool AT::isCommandFinished(void) const
{
    return mCommandFinished;
}

void AT::okReceived(void) const
{
    printf("%s OK\n", mName.data());
}
void AT::errorReceived(void) const
{
    printf("%s ERROR\n", mName.data());
}
AT::ReturnType AT::onResponseMatch(AT::ReceiveFunction&) const
{
    printf("Hello from %s\n", mName.data());
    return ReturnType::WAITING;
}

AT::ReturnType ATCmdOK_ERROR::onResponseMatch(AT::ReceiveFunction&) const
{
    if (mWaitingCmd) {
        mWaitingCmd();
        return ReturnType::FINISHED;
    }
    return ReturnType::ERROR;
}

std::array<char, ATParser::BUFFERSIZE> ATParser::ReceiveBuffer;
std::function<void(void)> ATParser::placeholder1;
std::function<void(void)> ATParser::placeholder2;

bool ATParser::parse(void) const
{
    auto allResponses = getAllResponses();

    size_t currentPos = 0;
    auto possibleResponses = allResponses;

    while (true) {
        if (mReceive(reinterpret_cast<uint8_t*>(ReceiveBuffer.data() + currentPos++), 1, mTimeout) != 1) {
            // timeout
            return false;
        }

        std::string_view currentData(ReceiveBuffer.data(), currentPos);

        Trace(ZONE_INFO, "Search area: %s\n", std::string(currentData.data(), currentData.length()).c_str());

        decltype(possibleResponses) sievedResponses;

        for (auto tup : possibleResponses) {
            auto str = std::get<0>(tup);
            if (currentData == str.substr(0, currentPos)) {
                sievedResponses.push_back(tup);
            }
        }
        possibleResponses = sievedResponses;

        if (possibleResponses.size() > 1) { continue; }

        if (possibleResponses.size() == 1) {
            //match
            auto match = std::get<0>(possibleResponses[0]);

            if (currentPos < match.length()) {
                continue;
            }
            auto onMatch = std::get<1>(possibleResponses[0]);
            Trace(ZONE_INFO, "MATCH: %s\n", match.data());

            switch (onMatch(mReceive)) {
            case AT::ReturnType::WAITING:
                mWaitingCmdOk = std::get<2>(possibleResponses[0]);
                mWaitingCmdError = std::get<3>(possibleResponses[0]);
                break;

            case AT::ReturnType::ERROR:
                Trace(ZONE_ERROR, "Error occured \r\n");
                break;

            case AT::ReturnType::FINISHED:
                break;
            }
        }

        //nothing found
        currentPos = 0;
        possibleResponses = allResponses;
        Trace(ZONE_INFO, "reload\n");
    }

    return true;
}
