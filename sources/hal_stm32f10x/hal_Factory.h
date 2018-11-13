// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_FACTORY_H_
#define SOURCES_PMD_FACTORY_H_

namespace hal
{
template<typename T>
const T& getFactory(void)
{
    static T instance;
    return instance;
}
//__attribute__ ((__constructor__)) advise GCC to call this function before main.
template<typename T>
inline void initFactory(void)
{
    getFactory<T>();
}

template<typename T>
class Factory
{
    Factory(void) = delete;

public:
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}

#endif /* SOURCES_PMD_FACTORY_H_ */
