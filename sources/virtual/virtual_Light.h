// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
