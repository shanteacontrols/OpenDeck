FROM ubuntu:23.10

ARG wget_args="-q --show-progress --progress=bar:force:noscroll"
ARG deps_dir=/opendeck-deps
ARG arduino_dir=${deps_dir}/arduino
ARG arduino_version=1.8.6
ARG user=ubuntu

RUN \
apt-get update && \
DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
alsa-utils \
avrdude \
bash-completion \
bsdmainutils \
bzip2 \
ca-certificates \
ccache \
clang \
clang-format \
clang-tidy \
cmake \
curl \
dfu-util \
g++ \
gcc \
gcc-arm-none-eabi \
gdb \
git \
git-lfs \
gpg-agent \
hub \
libgmock-dev \
libgoogle-glog-dev \
libgtest-dev \
libstdc++-arm-none-eabi-newlib \
lsb-release \
make \
nano \
ninja-build \
openssh-client \
software-properties-common \
srecord \
sudo \
tzdata \
unzip \
wget \
xz-utils

ENV CCACHE_DIR=/home/${user}/OpenDeck/ccache

RUN \
mkdir ${deps_dir}

RUN \
cd ${deps_dir} && \
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=${deps_dir} sh && \
./arduino-cli config init && \
./arduino-cli config set directories.data ${arduino_dir} && \
./arduino-cli core install arduino:avr@${arduino_version}

RUN \
cd ${deps_dir} && \
ARCH="$(dpkg --print-architecture)" && \
if [ "$ARCH" = "amd64" ]; then \
PACKAGE_VARIANT="amd64"; \
elif [ "$ARCH" = "arm64" ] || [ "$ARCH" = "aarch64" ]; then \
PACKAGE_VARIANT="arm64"; \
else \
echo "Unsupported architecture"; false; \
fi \
&& \
echo "Downloading dasel for $PACKAGE_VARIANT arch" && \
wget ${wget_args} https://github.com/TomWright/dasel/releases/download/v1.27.3/dasel_linux_${PACKAGE_VARIANT} -O dasel && \
chmod +x dasel

RUN \
echo "PATH=${deps_dir}:$(dirname $(find "$(pwd)" -type f -name "*avr-g++")):${PATH}" | tee -a /etc/profile.d/opendeck-deps.sh

# Disable password prompt for sudo commands
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Run everything below as $user
USER ${user}
WORKDIR /home/${user}

ADD .devcontainer/.git_branch_bash /home/${user}/.git_branch_bash

RUN tee -a /home/${user}/.bashrc <<EOF
alias mkc='make clean'
export MAKEFLAGS=-j$(nproc)
source ~/.git_branch_bash
source /etc/profile.d/opendeck-deps.sh
EOF