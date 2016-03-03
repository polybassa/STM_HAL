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

#ifndef SOURCES_PMD_LIGHTINTERFACE_H_
#define SOURCES_PMD_LIGHTINTERFACE_H_

namespace interface
{
typedef struct __attribute__((packed)) {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Color;

inline bool operator==(const Color& c1, const Color& c2)
{
    return c1.red == c2.red &&
           c1.green == c2.green &&
           c1.blue == c2.blue;
}

inline bool operator!=(const Color& c1, const Color& c2)
{
    return !(c1 == c2);
}

template<class T>
struct Light {
    enum Description {
        HEADLIGHT, BACKLIGHT, __ENUM__SIZE
    };

    Description getDescription(void) const
    {
        return static_cast<T const* const>(this)->mDescription;
    }

    void setColor(const Color& color) const
    {
        static_cast<T const* const>(this)->setColor(color);
    }
};
}

#endif /* SOURCES_PMD_LIGHTINTERFACE_H_ */
