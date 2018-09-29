FROM node:8.12.0-stretch

ENV EMSDK_VERSION=1.38.12

# install CMake and Boost
RUN apt-get update && \
    apt-get install -y cmake libboost-all-dev libstdc++6 \
    gcc-multilib g++-multilib unzip

# install and activate Emscripten SDK
WORKDIR /root
RUN git clone https://github.com/juj/emsdk.git
WORKDIR /root/emsdk
RUN ./emsdk install sdk-${EMSDK_VERSION}-64bit && \
    ./emsdk activate sdk-${EMSDK_VERSION}-64bit
ENV EMSCRIPTEN_PATH=/root/emsdk/emscripten
RUN echo "source /root/emsdk/emsdk_env.sh" > /root/.bashrc

# install boost 1.58 for Emscripten
WORKDIR /root
RUN git clone https://github.com/arielm/chronotext-boost
WORKDIR /root/chronotext-boost
RUN bash -c "source ./setup.sh"
#   build with the Boost modules that librime needs
RUN sed -i 's/LIBRARIES=.*/LIBRARIES="--with-system --with-filesystem --with-iostreams --with-regex --with-signals --with-locale"/' ./build-emscripten.sh
RUN bash -c "source ~/.bashrc; ./build-emscripten.sh"

# Patch the Emscripten CMake's Platform definition,
#   commenting out the following three lines:
#     - set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#     - set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#     - set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
# This is an workaround in order for emcmake to find the location of Boost
RUN sed -i 's/set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)/#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)/'  \
    ${EMSCRIPTEN_PATH}/${EMSDK_VERSION}/cmake/Modules/Platform/Emscripten.cmake && \
    sed -i 's/set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)/#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)/'  \
    ${EMSCRIPTEN_PATH}/${EMSDK_VERSION}/cmake/Modules/Platform/Emscripten.cmake && \
    sed -i 's/set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)/#set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)/'  \
    ${EMSCRIPTEN_PATH}/${EMSDK_VERSION}/cmake/Modules/Platform/Emscripten.cmake


RUN mkdir -p /root/workspace
WORKDIR /root/workspace

CMD ["bash"]