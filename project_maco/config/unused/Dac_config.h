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

#ifndef SOURCES_PMD_DAC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_DAC_CONFIG_DESCRIPTION_H_

enum Description {
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_DAC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_DAC_CONFIG_CONTAINER_H_

static constexpr std::array<const Dac, Dac::Description::__ENUM__SIZE> Container =
{ {
      Dac(Dac::Description::__ENUM_SIZE, 0, 0, {0, 0, 0, 0}, 0)
  } };

#endif /* SOURCES_PMD_DAC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_DAC_CONFIG_DESCRIPTION_H_ */
