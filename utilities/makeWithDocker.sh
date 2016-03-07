#!/bin/bash

eval "$(/usr/local/bin/docker-machine env default)"

/usr/local/bin/docker run -i  -v $(pwd)/..:/src -w /src/project --net="host"  polybassa/pmd:latest make "$@" 
