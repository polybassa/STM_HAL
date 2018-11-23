// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <algorithm>
#include <string_view>
#include <cstring>

#include "unittest.h"
#include "binascii.h"

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------

//--------------------------MOCKING--------------------------

//-------------------------TESTCASES-------------------------

int ut_hexlifyVector(void)
{
    TestCaseBegin();

    std::string instr(
                      "\xad\xfa\xdf\x12\x46\xa7\x88\xa8\x79\xa8\x99\xe9\xe7\xe0\xf0\xf9\x0c\x0c\x90\x09\xd0\x9d\xd6\x7d\x87\x89\x9d",
                      27);
    std::vector<char> src(instr.begin(), instr.end());
    std::vector<char> dst;

    hexlify(dst, src);

    CHECK(dst.size() == src.size() * 2);

    std::string resultstr = "ADFADF1246A788A879A899E9E7E0F0F90C0C9009D09DD67D87899D";
    std::vector<char> result(resultstr.begin(), resultstr.end());

    CHECK(dst == result)

    TestCaseEnd();
}

int ut_hexlifyVectorArray(void)
{
    TestCaseBegin();

    std::string instr(
                      "\xad\xfa\xdf\x12\x46\xa7\x88\xa8\x79\xa8\x99\xe9\xe7\xe0\xf0\xf9\x0c\x0c\x90\x09\xd0\x9d\xd6\x7d\x87\x89\x9d",
                      27);
    std::array<char, 27> src;
    std::memcpy(src.data(), instr.data(), src.size());
    std::vector<char> dst;

    hexlify(dst, src);

    CHECK(dst.size() == src.size() * 2);

    std::string resultstr = "ADFADF1246A788A879A899E9E7E0F0F90C0C9009D09DD67D87899D";
    std::vector<char> result(resultstr.begin(), resultstr.end());

    CHECK_MEMCMP(dst.data(), result.data(), result.size());

    TestCaseEnd();
}

int ut_hexlifyArray(void)
{
    TestCaseBegin();

    std::string instr(
                      "\xad\xfa\xdf\x12\x46\xa7\x88\xa8\x79\xa8\x99\xe9\xe7\xe0\xf0\xf9\x0c\x0c\x90\x09\xd0\x9d\xd6\x7d\x87\x89\x9d",
                      27);
    std::array<char, 27> src;
    std::memcpy(src.data(), instr.data(), src.size());
    std::array<char, 27 * 2> dst;

    hexlify(dst, src);

    CHECK(dst.size() == src.size() * 2);

    std::string resultstr = "ADFADF1246A788A879A899E9E7E0F0F90C0C9009D09DD67D87899D";
    std::vector<char> result(resultstr.begin(), resultstr.end());

    CHECK_MEMCMP(dst.data(), result.data(), result.size());

    TestCaseEnd();
}

int ut_hexlifyString(void)
{
    TestCaseBegin();

    std::string src(
                    "\xad\xfa\xdf\x12\x46\xa7\x88\xa8\x79\xa8\x99\xe9\xe7\xe0\xf0\xf9\x0c\x0c\x90\x09\xd0\x9d\xd6\x7d\x87\x89\x9d",
                    27);
    std::string dst;

    hexlify(dst, src);

    CHECK(dst.size() == src.size() * 2);

    std::string result = "ADFADF1246A788A879A899E9E7E0F0F90C0C9009D09DD67D87899D";

    CHECK(dst == result)

    TestCaseEnd();
}

int ut_hexlifyStringView(void)
{
    TestCaseBegin();

    std::string srcstr(
                       "\xad\xfa\xdf\x12\x46\xa7\x88\xa8\x79\xa8\x99\xe9\xe7\xe0\xf0\xf9\x0c\x0c\x90\x09\xd0\x9d\xd6\x7d\x87\x89\x9d",
                       27);
    std::string_view src = srcstr;
    std::string dst;

    hexlify(dst, src);

    CHECK(dst.size() == src.size() * 2);

    std::string result = "ADFADF1246A788A879A899E9E7E0F0F90C0C9009D09DD67D87899D";

    CHECK(dst == result)

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_hexlifyVector);
    RunTest(true, ut_hexlifyVectorArray);
    RunTest(true, ut_hexlifyArray);
    RunTest(true, ut_hexlifyString);
    RunTest(true, ut_hexlifyStringView);
    UnitTestMainEnd();
}
