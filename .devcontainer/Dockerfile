FROM ghcr.io/paradajz/zenv:4448e095d154e18822a8f86f53522c1e63dabf11

ARG project_dir=project
ARG jlink_version=V926
ARG tmp_deps_dir=/tmp/deps
ARG west_update_args="--narrow -o=--depth=1"

RUN \
echo '#!/bin/bash\necho not running udevadm "$@"' | sudo tee -a /usr/bin/udevadm && \
sudo chmod +x /usr/bin/udevadm && \
sudo apt-get update && \
sudo apt-get -y --no-install-recommends install \
build-essential \
libasound2-dev \
libfontconfig1 \
libfreetype6 \
libice6 \
libsm6 \
libssl-dev \
libtool \
libusb-dev \
libx11-6 \
libx11-xcb1 \
libxcb1 \
libxcb-icccm4 \
libxcb-image0 \
libxcb-keysyms1 \
libxcb-randr0 \
libxcb-render0 \
libxcb-render-util0 \
libxcb-shape0 \
libxcb-shm0 \
libxcb-sync1 \
libxcb-xfixes0 \
libxext6 \
libxkbcommon0 \
libxkbcommon-x11-0 \
libxrender1 \
pkg-config && \
sudo rm -rf /var/lib/apt/lists/*

RUN \
mkdir -p ${tmp_deps_dir} && \
cd ${tmp_deps_dir} && \
git clone https://github.com/gbevin/SendMIDI.git && \
cd SendMIDI/Builds/LinuxMakefile && \
make && \
sudo mv build/sendmidi /usr/local/bin && \
cd ${tmp_deps_dir} && \
git clone https://github.com/gbevin/ReceiveMIDI.git && \
cd ReceiveMIDI/Builds/LinuxMakefile && \
make && \
sudo mv build/receivemidi /usr/local/bin && \
cd ${tmp_deps_dir} && \
git clone https://github.com/PaulStoffregen/teensy_loader_cli.git && \
cd teensy_loader_cli && \
make && \
sudo mv teensy_loader_cli /usr/local/bin && \
cd ${tmp_deps_dir} && \
git clone https://github.com/arduino/BOSSA.git && \
cd BOSSA && \
make bossac && \
sudo mv bin/bossac /usr/bin/arduino-bossac && \
cd && \
rm -rf ${tmp_deps_dir}

RUN \
ARCH=$(uname -m); \
if [ "$ARCH" = "x86_64" ]; then \
PACKAGE_VARIANT="x86_64"; \
elif [ "$ARCH" = "aarch64" ]; then \
PACKAGE_VARIANT="arm64"; \
fi; \
wget --post-data="accept_license_agreement=accepted&submit=Download+software" https://www.segger.com/downloads/jlink/JLink_Linux_${jlink_version}_${PACKAGE_VARIANT}.deb -O jlink.deb && \
sudo dpkg -i jlink.deb && \
rm -f jlink.deb

RUN \
ARCH="$(dpkg --print-architecture)" && \
if [ "$ARCH" = "amd64" ]; then \
PACKAGE_VARIANT="x86_64"; \
elif [ "$ARCH" = "arm64" ]; then \
PACKAGE_VARIANT="aarch64"; \
else echo "Unsupported architecture"; false; \
fi; \
cd && \
wget ${wget_args} \
-O nrfutil \
"https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/executables/${PACKAGE_VARIANT}-unknown-linux-gnu/nrfutil" && \
chmod +x nrfutil && \
sudo mv nrfutil /usr/bin/

# Temporarily clone ST's fork of OpenOCD so that flashing of STM32H5 targets works
RUN \
sudo apt-get update && \
sudo apt-get -y --no-install-recommends install \
autoconf \
automake \
libftdi1-dev \
libhidapi-dev \
libjaylink-dev \
libusb-1.0-0-dev \
m4 && \
sudo rm -rf /var/lib/apt/lists/* && \
cd /home/ubuntu && \
git clone --branch openocd-cubeide-r7 https://github.com/STMicroelectronics/OpenOCD.git ST_OpenOCD && \
cd ST_OpenOCD && \
git submodule update --init --recursive && \
./bootstrap && \
./configure --prefix=/opt/toolchains/st-openocd --enable-stlink --enable-cmsis-dap --enable-jlink --enable-ftdi --enable-dummy && \
make -j$(nproc) && \
./src/openocd --version && \
sudo make install && \
/opt/toolchains/st-openocd/bin/openocd --version && \
cd && \
rm -rf /home/ubuntu/ST_OpenOCD

RUN nrfutil self-upgrade
RUN nrfutil install device

RUN mkdir -p ${ZEPHYR_WS}/${project_dir}
COPY --chown=ubuntu:ubuntu west.yml ${ZEPHYR_WS}/${project_dir}/west.yml
RUN \
. ${ZEPHYR_WS}/.venv/bin/activate && \
cd ${ZEPHYR_WS}/${project_dir} && \
west update ${west_update_args} && \
west packages pip --install && \
rm -rf /home/ubuntu/.cache/pip && \
cd ${ZEPHYR_WS} && \
rm -rf ${ZEPHYR_WS}/${project_dir}
