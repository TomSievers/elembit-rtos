FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    ca-certificates \
    git \
    build-essential \
    clang \
    cmake \
    ninja-build && \
    rm -rf /var/lib/apt/lists/*

USER root
