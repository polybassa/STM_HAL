#!/bin/bash

docker run -i  -v $(pwd)/..:/src -w /src/project_maco --net="host"  polybassa/pmd:latest ./configure 
