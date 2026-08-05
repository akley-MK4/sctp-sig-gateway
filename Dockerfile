FROM cpp-compiler:v1.0.0 AS builder

WORKDIR /workspace

ARG APP_NAME=sctp-sig-gateway

RUN mkdir $APP_NAME
COPY proto $APP_NAME/proto
COPY src $APP_NAME/src
COPY build.sh $APP_NAME/build.sh
COPY conanfile.txt $APP_NAME/conanfile.txt
COPY meson.build $APP_NAME/meson.build

RUN cd $APP_NAME && ./build.sh

FROM ubuntu:jammy

RUN apt-get update && apt-get install -y \
    curl \
    wget \
    unzip \
    locales \
    jq \
    htop \
    iputils-ping \
    net-tools \
    vim \
    && rm -rf /var/lib/apt/lists/*

ARG APP_PATH=/usr/local/bin/sctp-sig-gateway
ARG BUILD_PATH=/workspace/sctp-sig-gateway/build/gw

COPY --from=builder $BUILD_PATH $APP_PATH
RUN chmod +x $APP_PATH
RUN setcap cap_net_admin,cap_net_raw+ep $APP_PATH

ENTRYPOINT [$APP_PATH]