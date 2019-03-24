# STM_HAL
----------

Current Build Status: 

![status](https://travis-ci.org/polybassa/STM_HAL.svg?branch=master)

Setup: 
----------
* Install Docker: [Docker](https://www.docker.com)
* Go to project folder: `cd STM_HAL/utilities`
* Start build environment: `./enterBuildEnvironment.sh` (for the default project)

Build:
-----------
Inside the docker container, you can run
```
./configure
make <Target>
```

Alternative build ways:
-----------
If you want to build for example the Horrorscope project
```
cd utilities
./configureWithDocker project_XYZ
./makeWithDocker.sh project_XYZ <target>
```

Targets:
-----------
| Target	  | Description
|-----------------|--------------
| `test`          | build and execute unit tests        
| `firmware`      | build release firmware
| `debug_firmware`| build debug firmware
| `uncrustify`    | run source code beautifier
| `docu`          | build doxygen documentation


Debugging:
-----------

* Start `st-util` on your host machine
* Run `./connectGDB.sh` in your docker container
* Happy debugging
 

