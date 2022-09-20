FROM ubuntu:20.04

ENV CLANG_VERSION 12
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -y \
 && apt-get install -y \
    git \
    cmake \
    build-essential \
    llvm-${CLANG_VERSION}-dev \
    libclang-${CLANG_VERSION}-dev \
    clang-${CLANG_VERSION}
