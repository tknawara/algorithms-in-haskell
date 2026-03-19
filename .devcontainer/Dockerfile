FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

# Avoid prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# 1. Install prerequisites for adding PPAs and external repos
RUN apt-get update && apt-get install -y --no-install-recommends \
    software-properties-common \
    lsb-release \
    wget \
    gnupg \
    ca-certificates \
    curl

# 2. Add PPA for GCC 14 and install it + core build tools
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
    build-essential \
    gcc-14 \
    g++-14 \
    cmake \
    ninja-build \
    gdb \
    git \
    pkg-config \
    libssl-dev \
    zlib1g-dev \
    libzstd-dev


ENV VCPKG_ROOT=/usr/local/vcpkg
ENV PATH="${PATH}:${VCPKG_ROOT}"

RUN git clone https://github.com/microsoft/vcpkg.git ${VCPKG_ROOT} \
    && ${VCPKG_ROOT}/bootstrap-vcpkg.sh -disableMetrics

RUN chown -R vscode:vscode ${VCPKG_ROOT}
# 3. Install Clang 19 suite using the official LLVM script
# We ensure lsb-release is present so the script can detect 'noble' (24.04)
RUN wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh 19 all \
    && rm llvm.sh

# 4. Set up update-alternatives
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 --slave /usr/bin/g++ g++ /usr/bin/g++-14 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100 --slave /usr/bin/clang++ clang++ /usr/bin/clang++-19 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-19 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-19 100

USER vscode

RUN ${VCPKG_ROOT}/bootstrap-vcpkg.sh -disableMetrics