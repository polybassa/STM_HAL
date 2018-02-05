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

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------

//--------------------------MOCKING--------------------------

//-------------------------TESTCASES-------------------------

int ut_DeepSleep(void)
{
    TestCaseBegin();

    CHECK(true == true);
    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_DeepSleep);
    UnitTestMainEnd();
}
