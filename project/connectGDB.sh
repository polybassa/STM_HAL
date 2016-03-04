#!/bin/bash

(echo tar ext $(./getHostIP.sh):4242; cat) | arm-none-eabi-gdb
