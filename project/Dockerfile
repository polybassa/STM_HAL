FROM ubuntu
MAINTAINER Nils Weiss <n125.we155@googlemail.com>
LABEL Description="Docker image for building pmd projects"

# General dependencies
RUN apt-get update && apt-get install -y \
  build-essential \
  cmake \
  make \
  automake \
  doxygen \
  vim \
  git \
  software-properties-common \
  python-software-properties

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test 

RUN apt-get update && \
  apt-get install -y \
  gcc-5 \
  g++-5 \
  ggcov \
  lcov \
  uncrustify \
  cppcheck \
  openssl 

RUN apt-get update && \
  apt-get install -y \
  texlive \
  graphviz 

RUN apt-get update && \
  apt-get install -y \
  libssl-dev

# arm-none-eabi toolchain
RUN add-apt-repository ppa:terry.guo/gcc-arm-embedded && \
  apt-get update && \
  apt-get install -y gcc-arm-none-eabi

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 50
RUN update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-5 50

# Cleanup
RUN apt-get clean && \
  rm -rf /var/lib/apt
