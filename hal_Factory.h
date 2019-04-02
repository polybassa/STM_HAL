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
    Factory(Factory &&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory &&) = delete;
};
}

#endif /* SOURCES_PMD_FACTORY_H_ */
