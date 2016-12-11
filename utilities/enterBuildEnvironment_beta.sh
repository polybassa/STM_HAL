#!/bin/bash

docker run -i -t -v $(pwd)/..:/src -w /src/project_beta --net="host"  polybassa/pmd:latest /bin/bash 
