/* Copyright (C) 2015  Nils Weiss, Alexander Strobl
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

#ifndef SOURCES_PMD_TEMPERATURESENSORINTERFACE_H_
#define SOURCES_PMD_TEMPERATURESENSORINTERFACE_H_

namespace interface
{
template<class T>
struct TemperatureSensor {
    enum Description {
        INTERNAL, BATTERY, MOTOR, FET, __ENUM__SIZE
    };

    Description getDescription(void) const
    {
        return static_cast<T const* const>(this)->mDescription;
    }

    float getTemperature(void) const
    {
        return static_cast<T const* const>(this)->getTemperature();
    }
};
}

#endif /* SOURCES_PMD_TEMPERATURESENSORINTERFACE_H_ */
