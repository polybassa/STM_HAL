#!/bin/bash

if [ $# -eq 0 ]
  then
    echo "No arguments supplied. Please provide a project directory."
    exit 1
fi

if [ -z "$1" ]
  then
    echo "No arguments supplied. Please provide a project directory."
    exit 1
fi

if [[ `basename $(pwd)` == "utilities" ]]
  then
	if [ -d "../$1" ]
	  then
	    docker run -i  -v $(pwd)/..:/src -w /src/$1 --net="host"  polybassa/pmd:latest ./configure 
	  else
	    echo "Make sure you run this script from inside the utilities folder."
	    echo "Example: ./configureWithDocker.sh project_tunnel"
	    exit 1 
	fi
else
  echo "Make sure you run this script from inside the utilities folder."
  echo "Example: ./configureWithDocker.sh project_tunnel"
  exit 1 
fi