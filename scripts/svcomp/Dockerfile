FROM ubuntu:26.04
ENV DEBIAN_FRONTEND=noninteractive
# Single RUN, such that the deleted package lists do not remain in an earlier layer
RUN dpkg --add-architecture i386 \
 && apt-get update \
 && apt-get install -y \
       tzdata \
       clang \
       clang-format \
       clang-tidy \
       g++-multilib \
       gcc \
       lcov \
       libc6-dev-i386 \
       libclang-cpp-dev \
       libclang-dev \
       libflint-dev \
       libgmp-dev \
       libgomp1 \
       libmpfr-dev \
       libmpfr6 \
       llvm \
       llvm-dev \
       make \
       openjdk-11-jre-headless \
       openjdk-17-jdk-headless \
       openjdk-17-jre-headless \
       openjdk-21-jre-headless \
       openjdk-25-jre-headless \
       openjdk-8-jdk-headless \
       openjdk-8-jre-headless \
       python3 \
       python3-bitstring \
       python3-clang \
       python3-docker \
       python3-graphviz \
       python3-jsonschema \
       python3-lxml \
       python3-pycparser \
       python3-requests \
       python3-tqdm \
       python3-yaml \
       zlib1g-dev \
       cmake \
 && ln -fs /usr/share/zoneinfo/Europe/Berlin /etc/localtime \
 && dpkg-reconfigure --frontend noninteractive tzdata \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /lachesis

COPY ../../src /lachesis/src
COPY ../../llvm-pass /lachesis/llvm-pass
COPY ../../CMakeLists.txt /lachesis/
COPY docker_build.sh /lachesis/

ENTRYPOINT [ "./docker_build.sh" ]