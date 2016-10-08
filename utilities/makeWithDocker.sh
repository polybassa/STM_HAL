#!/bin/bash

/usr/local/bin/docker run -i  -v $(pwd)/..:/src -w /src/project --net="host"  polybassa/pmd:latest make "$@" 
