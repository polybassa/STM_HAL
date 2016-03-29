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

#ifndef SOURCES_PMD_BATTERYINTERFACE_H_
#define SOURCES_PMD_BATTERYINTERFACE_H_

namespace interface
{
struct Battery {
    virtual float getTemperature(void) const {return 0.0; };

    virtual float getVoltage(void) const {return 0.0; };

    virtual float getCurrent(void) const {return 0.0; };

    virtual float getPower(void) const {return 0.0; };
    virtual ~Battery(void) {};
};
}

#endif /* SOURCES_PMD_BATTERYINTERFACE_H_ */
