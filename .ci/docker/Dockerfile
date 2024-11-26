# Use Ubuntu 22.04 as the base image
FROM registry.esss.lu.se/ecdc/ess-dmsc/build-nodes/ubuntu2204:1.0 AS builder

USER root

# Set up project directory and copy necessary files
WORKDIR /ess/ecdc/daqlite
COPY src ./src
COPY cmake ./cmake
COPY configs ./configs
COPY conanfile.txt ./conanfile.txt
COPY CMakeLists.txt ./CMakeLists.txt

# Unset proxy variables
RUN unset https_proxy http_proxy

# Build and install dependencies
RUN mkdir build && cd build && \
    conan remote add --insert 0 --force ecdc-conan-external https://artifactory.esss.lu.se/artifactory/api/conan/ecdc-conan-external && \
    conan install .. && \
    cmake .. && \
    make


FROM ubuntu:22.04

# Copy the built project from the previous stage
COPY --from=builder /ess/ecdc/daqlite/build/bin /ess/ecdc/daqlite/bin
COPY --from=builder /ess/ecdc/daqlite/build/lib /ess/ecdc/daqlite/lib

# Install necessary packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        tightvncserver \
        xfce4 \
        xfce4-goodies \
        qtcreator \
        qtbase5-dev \
        qt5-qmake \
        vim-tiny \
        x11-xserver-utils \
        gedit \
        xfonts-base \
        xfonts-75dpi \
        dbus-x11 \
        cmake \
        curl \
        git && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV USER=root
ENV DISPLAY=:1
ENV VNC_PASSWORD=password
ENV VNC_RESOLUTION=1280x800
ENV LD_LIBRARY_PATH="/ess/ecdc/daqlite/lib:${LD_LIBRARY_PATH}"
ENV DAQLITE_HOME="/ess/ecdc/daqlite"
ENV DAQLITE_CONFIG="/ess/ecdc/daqlite/configs"
ENV DAQLITE_PRODUCTION=true
ENV PATH="/ess/ecdc/daqlite/scripts:${PATH}"

# Create directories
RUN mkdir -p /ess/ecdc/daqlite /ess/ecdc/daqlite/configs

# Set up Kafka configuration
RUN echo "{}" > /ess/ecdc/daqlite/configs/kafka-config-daqlite.json

# Copy startup script
COPY .ci/docker/Dockerfile_entrypoint.sh /ess/ecdc/daqlite/entrypoint.sh
RUN chmod +x /ess/ecdc/daqlite/entrypoint.sh

# Expose VNC port
EXPOSE 5901

# Start the container with the startup script
CMD ["/ess/ecdc/daqlite/entrypoint.sh"]
