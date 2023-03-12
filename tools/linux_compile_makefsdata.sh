#!/bin/sh

# This compiles makefsdata for Linux statically linking against musl
# Make sure that you have the required packages installed (for Ubuntu these are: musl and musl-tools)

musl-gcc $PICO_SDK_PATH/lib/lwip/src/apps/http/makefsdata/makefsdata.c --no-pie -static -o makefsdata -I$PICO_SDK_PATH/lib/lwip/src/include -I$PICO_SDK_PATH/lib/lwip/contrib/ports/unix/lib -I$PICO_SDK_PATH/lib/lwip/contrib/ports/unix/port/include
