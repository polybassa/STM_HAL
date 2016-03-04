#!/bin/bash

arm-none-eabi-gdb -ex "tar ext $(./../utilities/getHostIP.sh):4242"
