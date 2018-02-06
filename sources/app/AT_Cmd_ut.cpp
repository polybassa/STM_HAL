/* Copyright (C) 2016  Nils Weiss
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

#include "unittest.h"
#include "AT_Cmd.h"
#include "AT_Parser.h"

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------

//--------------------------MOCKING--------------------------

void os::ThisTask::sleep(const std::chrono::milliseconds ms) {}

//-------------------------TESTCASES-------------------------

int ut_ATParser(void)
{
    TestCaseBegin();

    static std::string testString = "\r\nRES\rRESP\nRESP2\rOK\r\nReSP\r\nREsp3\rOK\r\nPES";
    static auto pos = testString.begin();

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
    [&](uint8_t * data, const size_t length, std::chrono::milliseconds)->size_t {
        size_t i = 0;
        for ( ; i < length && pos != testString.end(); i++) {
            *data = *pos++;
        }
        return i;
    };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
        [] (std::string_view, std::chrono::milliseconds)->size_t {
        return 0;
    };

    app::AtParser parser(send, recv);

    parser.parse();

    CHECK(true == true);
    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_ATParser);
    UnitTestMainEnd();
}
