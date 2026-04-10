FROM ghcr.io/paradajz/zenv:7a7ca0f53c37c8240c7d39d1773efdb2f7095928

ARG jlink_version=V926

RUN \
echo '#!/bin/bash\necho not running udevadm "$@"' | sudo tee -a /usr/bin/udevadm && \
sudo chmod +x /usr/bin/udevadm && \
sudo apt-get update && \
sudo apt-get -y --no-install-recommends install \
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
libxrender1

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
