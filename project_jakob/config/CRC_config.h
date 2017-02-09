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

#ifndef SOURCES_PMD_CRC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_CRC_CONFIG_DESCRIPTION_H_

enum Description {
    SYSTEM_CRC, __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_CRC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_CRC_CONFIG_CONTAINER_H_

static constexpr std::array<const Crc, Crc::__ENUM__SIZE> Container =
{ {
      Crc(Crc::SYSTEM_CRC, CRC_PolSize_8, CRC_ReverseInputData_No, false, 0x00, 0x83)
  } };

#endif /* SOURCES_PMD_CRC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_CRC_CONFIG_DESCRIPTION_H_ */
