FROM ghcr.io/paradajz/zenv:c5553452a12edb73f5b9391123460944a1a3ecd5

ARG jlink_version=V926
ARG tmp_deps_dir=/tmp/deps

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
pkg-config

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

RUN nrfutil self-upgrade
RUN nrfutil install device
