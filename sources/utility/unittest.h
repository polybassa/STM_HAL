// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss, Patrick Bruenn
 */

#pragma once

#include <stdio.h>
#include "trace.h"

#define CHECK(EXPRESSION) if (!(EXPRESSION)) { \
        errors++; \
        printf("ERROR %s:%d: %s() CHECK(" # EXPRESSION ") failed\n", __FILE__, __LINE__, __FUNCTION__); \
}

#define CHECK_MEMCMP(BUFFER, REF_DATA, REF_SIZE) { \
        CHECK(0 == memcmp(BUFFER, REF_DATA, REF_SIZE)); \
}

#define CHECK_NOT_MEMCMP(BUFFER, REF_DATA, REF_SIZE) { \
        CHECK(0 != memcmp(BUFFER, REF_DATA, REF_SIZE)); \
}

#define NOT_IMPLEMENTED(X) { \
        printf("ERROR %s:%d: %s() NOT IMPLEMENTED\n", __FILE__, __LINE__, __FUNCTION__); \
        return 1; \
}

#define RunTest(RUN, FUNC) { \
        if (RUN) { \
            size_t _errors = FUNC(); \
            if (_errors != 0) { \
                printf(# FUNC "() run with %d errors\n", (int)_errors); \
                numErrors += _errors; \
            } \
            numTests++; \
        } \
        else { \
            numSkipped++; \
        } \
}

#define TestCaseBegin(X) size_t errors = 0
#define TestCaseEnd(X) return errors

#define UnitTestMainBegin(X) int numErrors = 0; int numSkipped = 0; int numTests = 0
#define UnitTestMainEnd(X) do { \
        printf("%36s run %2d Tests (%2d skipped | %2d errors)\n", __FILE__, numTests, numSkipped, numErrors); \
        return numErrors; \
} while (0)
