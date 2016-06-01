#!/bin/bash

docker run -i  -v $(pwd)/..:/src -w /src/project_Bootloader --net="host"  polybassa/pmd:latest make "$@" 
