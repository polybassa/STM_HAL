#******************************************************************************
# Copyright (C) 2014 Nils Weiss, Patrick Bruenn.
#
# This file is part of WyLight.
#
# WyLight is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# WyLight is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with WyLight.  If not, see <http://www.gnu.org/licenses/>.

#******************************************************************************

#******************************************************************************
#
# Makefile - Rules for building the WyLight bootloader.
#
#
#*****************************************************************************
PRJ_PREFIX=HAL_
PRJ_NAME=${PRJ_PREFIX}$(MAKECMDGOALS)
FLAVOUR=$(MAKECMDGOALS)
PRJ_PATH=/home/nils/projects/STM_HAL/project_maco

ifneq (,$(filter debug_firmware firmware,$(MAKECMDGOALS)))
PREFIX=arm-none-eabi-
endif

# Tools
CPPCHECKER=not_found
UNCRUSTIFY=/usr/bin/uncrustify
ARM_GCC=/usr/bin/arm-none-eabi-gcc
ARM_GPP=/usr/bin/arm-none-eabi-g++
ARM_OBJCOPY=/usr/bin/arm-none-eabi-objcopy
ARM_AR=/usr/bin/arm-none-eabi-ar
ARM_LD=/usr/bin/arm-none-eabi-ld
ARM_SIZE=/usr/bin/arm-none-eabi-size
GCOV=/usr/bin/gcov
LCOV=/usr/bin/lcov
GENHTML=/usr/bin/genhtml

DEVICE=STM32F10X_HD

all: 
	make test 
	make clean
	make firmware

# Include the common make definitions.
include makedefs

pre-build:


post-build: 
	openssl sha256 -binary ${BINDIR}/${PRJ_NAME}.bin > ${BINDIR}/${PRJ_NAME}.sha

firmware: pre-build ${OBJDIR} ${BINDIR} ${BINDIR}/${PRJ_NAME}.elf post-build
	@echo NON-DEBUG BUILD

debug_firmware: pre-build ${OBJDIR} ${BINDIR} ${BINDIR}/${PRJ_NAME}.elf post-build
	@echo DEBUG BUILD
		
docu:
	@/usr/bin/doxygen ../docs/Doxyfile
	
check:
	@not_found --enable=all --inconclusive -I ./config/ -I ../sources/hal/ -I ../sources/dev/ -I ../sources/app/ -I ../sources/utility/ -I ../sources/interface/ -I ../sources/os/ -I ../libraries/STM32F30x_StdPeriph_Driver/inc/ --std=posix --quiet .

ENDINGS=c h cpp
CODE_DIRS=../sources config apps .
filelist:
	@rm -f $@.txt
	@$(foreach END, ${ENDINGS}, $(foreach DIR, ${CODE_DIRS}, find ./$(DIR) -name "*.$(END)" >> $@.txt;))
	@echo "Created $@.txt"

uncrustify: filelist
	${UNCRUSTIFY} -F filelist.txt --no-backup -c ../utilities/uncrustify.cfg


# The rule to clean out all the build products.
clean:
	@echo "Clean objectfiles"
	@rm -rf ${OBJDIR} ${wildcard *~}
	@make ${OBJDIR}
	@rm -rf ${COVERAGEDIR}

# The rule to clean out all the build products.
clean-all:
	@echo "Clean objectfiles and binarys"
	@rm -rf ${OBJDIR} ${wildcard *~}
	@rm -rf ${BINDIR} ${wildcard *~}
	@rm -f ${FW_FILE}

# The rule to create the target directories.
${OBJDIR}:
	@mkdir -p ${OBJDIR}

${BINDIR}:
	@mkdir -p ${BINDIR}

ifneq (,$(filter debug_firmware firmware,$(MAKECMDGOALS)))
include Makefile.prj
endif

ifneq (,$(filter test,$(MAKECMDGOALS)))
include Makefile.test
endif

