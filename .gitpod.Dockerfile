FROM gitpod/workspace-full-vnc

USER gitpod

RUN sudo apt-get -q update && \
    sudo apt-get install -yq libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev cloc

# Temporary fix for a broken clang package:
# https://github.com/gitpod-io/gitpod/issues/1694
ENV CC=clang-11
