FROM gitpod/workspace-full-vnc

USER gitpod

# Install custom tools, runtime, etc. using apt-get
# For example, the command below would install "bastet" - a command line tetris clone:

RUN sudo apt-get -q update && \
    sudo apt-get install -yq libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev clang clang-format gcc make cloc
