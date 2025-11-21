FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    cmake \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-venv \
    wget \
    nano \
    curl \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

ENV PICO_SDK_PATH=/opt/pico-sdk

RUN git clone --recursive https://github.com/raspberrypi/pico-sdk.git --branch 2.1.1 "$PICO_SDK_PATH" \
    && cd "$PICO_SDK_PATH" \
    && git submodule update --init

RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - && \
    apt-get install -y nodejs

RUN apt-get update && apt-get install -y \
    libusb-1.0-0-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# avoid git safe directory checks for the workspace
RUN git config --global --add safe.directory /workspace
RUN git config --global --add safe.directory /workspace/lib/pico_pio_usb
RUN git config --global --add safe.directory /workspace/lib/tinyusb