#!/bin/bash
docker run -i -t -v $(pwd)/..:/src -w /src/project polybassa/pmd:latest /bin/bash 

