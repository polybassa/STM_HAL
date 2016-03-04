#!/bin/bash

arm-none-eabi-gdb -ex "tar ext $(./getHostIP.sh):4242"
