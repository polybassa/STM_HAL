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

#ifndef LIBRARIES_EMPL_PMD_EMPL_H_
#define LIBRARIES_EMPL_PMD_EMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

int empl_i2c_read(unsigned char  Address,
                  unsigned char  RegisterAddr,
                  unsigned short RegisterLen,
                  unsigned char* RegisterValue);
int empl_i2c_write(unsigned char        Address,
                   unsigned char        RegisterAddr,
                   unsigned short       RegisterLen,
                   const unsigned char* RegisterValue);

int empl_get_ms(unsigned long* count);
int empl_delay_ms(unsigned long);

#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_EMPL_PMD_EMPL_H_ */
