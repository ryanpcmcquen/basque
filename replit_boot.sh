#!/bin/sh

echo "Checking for SDL ...";

if [ -z "$(which sdl-config)" ] || [ -z "$(dpkg-query -l | grep sdl2)" ]; then
    install-pkg libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-mixer-dev \
        libsdl2-ttf-dev
fi
