FROM ubuntu
MAINTAINER Nils Weiss <n125.we155@googlemail.com>
LABEL Description="Docker image for building pmd projects"

# General dependencies
RUN apt-get update && apt-get install -y \
  build-essential \
  git \
  subversion \
  cmake \
  make \
  automake \
  python-setuptools \
  ninja-build \
  python-dev \
  libffi-dev \
  libssl-dev \
  software-properties-common \
  python-software-properties \
  vim 

RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test 

RUN apt-get update && \
  apt-get install -y \
  gcc-5 \
  g++-5 \
  ggcov \
  lcov \
  uncrustify \
  cppcheck \
  openssl \
  texlive-full \
  graphviz 

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
