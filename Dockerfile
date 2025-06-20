FROM ubuntu:22.04 
ENV DEBIAN_FRONTEND=noninteractive
SHELL [ "/bin/bash", "-c" ]

# Setup some important paths for the structures
ARG PROJ_DIR=/usr/workdir
ARG EXT_DIR=${PROJ_DIR}/external
ARG RTOS_DIR=${EXT_DIR}/rtos
ARG SDK_DIR=${EXT_DIR}/sdk
ARG LIB_DIR=${EXT_DIR}/lib
ARG CASE_DIR=${PROJ_DIR}/experiments/case_study
ARG PATCH_DIR=${EXT_DIR}/patches 
ARG TOOL_DIR=${PROJ_DIR}/tools
ARG ARM_PATH=${TOOL_DIR}/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi

WORKDIR /usr/workdir

# Copy over important files
COPY . .

# Install required packages
RUN     apt-get update \
    &&  apt-get upgrade -y \
    &&  apt-get install -y --no-install-recommends \
            git \
            cmake \
            curl \
            ninja-build \
            gperf \
            ccache \
            dfu-util \
            device-tree-compiler \
            wget \
            python3-dev \
            python3-pip \
            python3-setuptools \
            python3-tk \
            python3-wheel \
            python3-venv \
            xz-utils file \
            make \
            gcc \
            gcc-multilib \
            g++-multilib \
            libsdl2-dev \
            libmagic1 \
            bzip2 \
            build-essential \
            nano \
            systemd-sysv \
            tmux \
            udev \
            unzip \
            usbutils \
    &&  apt-get clean

# Install python packages
COPY requirements.txt ./
RUN  pip3 install --no-cache-dir -r requirements.txt

# Install arm-none-eabi-gcc
WORKDIR ${TOOL_DIR}
RUN     wget https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz \
    &&  tar -xvf arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz
ENV     PATH="$PATH:${ARM_PATH}/bin/"

# Install SEGGER tools
WORKDIR ${TOOL_DIR}
RUN     curl -d "accept_license_agreement=accepted&submit=Download+software" -X POST -O "https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.tgz" \
    &&  mkdir -p /opt/SEGGER \
    &&  cd /opt/SEGGER \
    &&  tar xzf ${TOOL_DIR}/JLink_*.tgz \
    &&  mv /opt/SEGGER/JLink_Linux_V* /opt/SEGGER/JLink

# Install nrfjprog tools
WORKDIR ${TOOL_DIR}
RUN     wget https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x/10-24-2/nrf-command-line-tools_10.24.2_amd64.deb \
    &&  apt install -y ./nrf-command-line-tools_10.24.2_amd64.deb 

# Install amazon-free-rtos
WORKDIR ${RTOS_DIR}
RUN     git clone https://github.com/aws/amazon-freertos.git amazon-freertos \
    &&  cd amazon-freertos \
    &&  git checkout 1fc5e966a0f8e541c0942bd02bb3504a4a6be9c6 \
    &&  git apply ${PATCH_DIR}/amazon-freertos.patch

# Install FreeRTOS
WORKDIR ${RTOS_DIR}
RUN     git clone --recursive https://github.com/FreeRTOS/FreeRTOS.git FreeRTOS \
    &&  cd FreeRTOS \
    &&  git checkout -f f7bc6297ca132b9acc98e1346cc11ae1c4e9bc6d \
    &&  git apply ${PATCH_DIR}/FreeRTOS.patch 

# Install FreeRTOS-Kernel
WORKDIR ${RTOS_DIR}
RUN     git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS-Kernel \
    &&  cd FreeRTOS-Kernel \
    &&  git checkout c7a9a01c94987082b223d3e59969ede64363da63 \
    &&  git apply ${PATCH_DIR}/FreeRTOS-Kernel.patch

# Install picotcp
WORKDIR ${LIB_DIR}
RUN     git clone https://github.com/tass-belgium/picotcp.git picotcp \
    &&  cd picotcp \
    &&  git checkout 46120abecc9fb79f7dfb2cc8192341a42e40fc8b \
    &&  git apply ${PATCH_DIR}/picotcp.patch \
    &&  make CROSS_COMPILE=arm-none-eabi- ARCH=cortexm4-hardfloat 
    
# Install crazyflie-firmware
WORKDIR ${CASE_DIR}
RUN     git clone https://github.com/bitcraze/crazyflie-firmware/ crazyflie-firmware --recursive \
    &&  cd crazyflie-firmware \
    &&  git checkout 7a0ed44bb28440770702d95cd2ac4862e1ef5071 \
    &&  git apply ${PATCH_DIR}/crazyflie-firmware.patch \
    &&  cp ${CASE_DIR}/HOTPATCH.ld ${CASE_DIR}/crazyflie-firmware/tools/make/F405/linker/HOTPATCH.ld \
    &&  cp ${CASE_DIR}/hp_config.h ${CASE_DIR}/crazyflie-firmware/src/config/hp_config.h

# Unpack the nRF SDK
WORKDIR ${SDK_DIR}
RUN     curl -d "ids=A9F616524B054D2C85188560FA13799D&filename=DeviceDownload" -X POST -O "https://www.nordicsemi.com/api/sitecore/Products/MedialibraryZipDownload2" \
    &&  unzip "MedialibraryZipDownload2" \
    &&  unzip "nRF5_SDK_17.1.0_ddde560.zip" \
    &&  rm ${SDK_DIR}/nRF5_SDK_17.1.0_ddde560/components/toolchain/gcc/Makefile.posix \
    &&  echo -e "GNU_INSTALL_ROOT ?= ${ARM_PATH}/bin/\nGNU_VERSION ?= 14.2.1\nGNU_PREFIX ?= arm-none-eabi" >> ${SDK_DIR}/nRF5_SDK_17.1.0_ddde560/components/toolchain/gcc/Makefile.posix



# Zephyr Installation
WORKDIR ${RTOS_DIR}/zephyr
RUN     wget https://apt.kitware.com/kitware-archive.sh \
    &&  bash kitware-archive.sh

RUN     python3 -m venv ${RTOS_DIR}/zephyr/zephyrproject/.venv \
    &&  source ${RTOS_DIR}/zephyr/zephyrproject/.venv/bin/activate \
    &&  pip install pyyaml==6.0 \
    &&  pip install west \
    &&  west init ${RTOS_DIR}/zephyr/zephyrproject \
    &&  cd ${RTOS_DIR}/zephyr/zephyrproject \
    &&  west update \
    &&  west zephyr-export \
    &&  west packages pip --install \
# apply the patches
    &&  cd ${RTOS_DIR}/zephyr/zephyrproject/zephyr \
    &&  git checkout e60da1bd640a37370870a83277142dd560f1fb8d \ 
    &&  west -v update \ 
    &&  git apply ${PATCH_DIR}/zephyr.patch

# install the zephyr sdk (version 0.16.8)
WORKDIR ${SDK_DIR}
RUN     wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-x86_64.tar.xz \
    &&  tar xvf zephyr-sdk-0.16.8_linux-x86_64.tar.xz \
    &&  cd zephyr-sdk-0.16.8 \
    &&  ./setup.sh -t arm-zephyr-eabi \
    &&  ./setup.sh -h \
    &&  ./setup.sh -c

# reset the workdir
WORKDIR ${PROJ_DIR}