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

#pragma once

#include <cstdint>
#include "interface_Light.h"

namespace virt
{
struct Light {
    Light() = delete;
    Light(const Light&) = delete;
    Light(Light&&) = default;
    Light& operator=(const Light&) = delete;
    Light& operator=(Light&&) = delete;

    inline interface ::Color getColor(void) const;
    inline void setColor(const interface ::Color& color);

    const enum interface::Light::Description mDescription;

    Light(const enum interface::Light::Description& desc) :
        mDescription(desc) {}

private:
    interface ::Color mColor;
};
}

interface ::Color virt::Light::getColor(void) const
{
    return mColor;
}

void virt::Light::setColor(const interface ::Color& color)
{
    mColor.red = color.red;
    mColor.green = color.green;
    mColor.blue = color.blue;
}
