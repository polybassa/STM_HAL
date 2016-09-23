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

#ifndef SOURCES_PMD_REALTIMEDEBUGINTERFACE_H_
#define SOURCES_PMD_REALTIMEDEBUGINTERFACE_H_

#include "SEGGER_RTT.h"

namespace dev
{
struct RealTimeDebugInterface
{
	inline RealTimeDebugInterface(void) {SEGGER_RTT_ConfigUpBuffer(0,nullptr, nullptr, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM); }
	RealTimeDebugInterface(const RealTimeDebugInterface&) = delete;
	RealTimeDebugInterface(RealTimeDebugInterface &&) = delete;
	RealTimeDebugInterface& operator=(const RealTimeDebugInterface&) = delete;
	RealTimeDebugInterface& operator=(RealTimeDebugInterface &&) = delete;

	template<typename ...Params>
	void printf(Params&&... params){
		SEGGER_RTT_printf(0, std::forward<Params>(params)...);
	}

};
}

#endif /* SOURCES_PMD_REALTIMEDEBUGINTERFACE_H_ */
