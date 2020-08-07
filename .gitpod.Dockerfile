FROM gitpod/workspace-full-vnc

USER gitpod

RUN sudo apt-get -q update && \
    sudo apt-get install -yq libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev clang-10 clang-format-10 clangd-10 gcc make cloc
