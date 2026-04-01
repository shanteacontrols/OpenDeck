FROM ghcr.io/paradajz/zenv:b521042e7fc04cf76ad7a2b47a913b8396e000c0

ARG arduino_dir=/opt/toolchains/arduino
ARG arduino_version=1.8.6

RUN \
sudo apt-get update && \
sudo apt-get install --no-install-recommends -y \
avrdude \
libgoogle-glog-dev \
unzip \
libfontconfig1 \
libfreetype6 \
libice6 \
libsm6 \
libssl-dev \
libtool \
libx11-6 \
libx11-xcb1 \
libxcb-icccm4 \
libxcb-image0 \
libxcb-keysyms1 \
libxcb-randr0 \
libxcb-render-util0 \
libxcb-render0 \
libxcb-shape0 \
libxcb-shm0 \
libxcb-sync1 \
libxcb-xfixes0 \
libxcb1 \
libxext6 \
libxkbcommon-x11-0 \
libxkbcommon0 \
libxrender1 && \
sudo rm -rf /var/lib/apt/lists/*

RUN \
sudo mkdir -p ${arduino_dir} && \
sudo chmod -R 777 ${arduino_dir} && \
cd ${arduino_dir} & \
wget -qO- https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=${arduino_dir} sh && \
${arduino_dir}/arduino-cli config init && \
${arduino_dir}/arduino-cli config set directories.data ${arduino_dir} && \
${arduino_dir}/arduino-cli core install arduino:avr@${arduino_version}

RUN \
ARCH="$(dpkg --print-architecture)" && \
if [ "$ARCH" = "amd64" ]; then \
PACKAGE_VARIANT="x86_64"; \
elif [ "$ARCH" = "arm64" ]; then \
PACKAGE_VARIANT="arm64"; \
else echo "Unsupported architecture"; false; \
fi; \
cd /tmp && \
wget \
-O nrfclt.deb \
https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x/10-24-2/nrf-command-line-tools_10.24.2_${ARCH}.deb && \
sudo dpkg -i nrfclt.deb && \
rm nrfclt.deb && \
echo '#!/bin/bash\necho not running udevadm "$@"' | sudo tee -a /usr/bin/udevadm && \
sudo chmod +x /usr/bin/udevadm && \
sudo apt-get install --no-install-recommends -y \
/opt/nrf-command-line-tools/share/JLink_Linux_V794e_${PACKAGE_VARIANT}.deb --fix-broken

ENV ARM_TOOLCHAIN_PATH=$ZEPHYR_SDK_INSTALL_DIR/arm-zephyr-eabi/bin
ENV ARM_TOOLCHAIN_PREFIX=arm-zephyr-eabi-

ENV AVR_TOOLCHAIN_PATH=${arduino_dir}/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7/bin
ENV AVR_TOOLCHAIN_PREFIX=avr-