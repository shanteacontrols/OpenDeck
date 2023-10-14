FROM ubuntu:23.04

ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll"

RUN \
apt-get update && \
DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
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
gcc-multilib \
gdb \
git \
git-lfs \
gpg-agent \
lsb-release \
make \
software-properties-common \
srecord \
sudo \
tzdata \
unzip \
wget \
xz-utils \
&& rm -rf /var/lib/apt/lists/*

RUN \
mkdir /opendeck-deps

RUN \
cd /opendeck-deps && \
wget ${WGET_ARGS} https://github.com/TomWright/dasel/releases/download/v1.27.3/dasel_linux_amd64 && \
chmod +x dasel_linux_amd64 && \
mv $(pwd)/dasel_linux_amd64 /usr/local/bin/dasel

RUN \
cd /opendeck-deps && \
wget ${WGET_ARGS} https://downloads.arduino.cc/arduino-1.8.19-linux64.tar.xz -O arduino.tar.xz && \
tar -xf arduino.tar.xz && \
rm arduino.tar.xz && \
cd arduino-1.8.19 && \
rm -rf \
java \
examples \
lib \
libraries \
reference \
tools-builder \
arduino \
arduino-builder \
arduino-linux-setup.sh \
install.sh \
revisions.txt \
uninstall.sh && \
cp hardware/tools/avr/etc/avrdude.conf /etc/avrdude.conf

RUN \
cd /opendeck-deps && \
wget ${WGET_ARGS} https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
tar -xf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
rm gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2

RUN \
cd /opendeck-deps && \
wget ${WGET_ARGS} https://github.com/github/hub/releases/download/v2.14.2/hub-linux-amd64-2.14.2.tgz && \
tar -xf hub-linux-amd64-2.14.2.tgz && \
rm hub-linux-amd64-2.14.2.tgz && \
cp hub-linux-amd64-2.14.2/bin/hub /usr/bin/hub && \
rm -rf hub-linux-amd64-2.14.2

RUN \
cd && \
wget ${WGET_ARGS} https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip && \
unzip ninja-linux && \
rm ninja-linux.zip && \
chmod a+x ninja && \
mv ninja /usr/local/bin/ninja

ENV PATH="/opendeck-deps/arduino-1.8.19/hardware/tools/avr/bin:/opendeck-deps/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH"

RUN \
cd /opendeck-deps && \
git clone https://github.com/google/googletest.git -b release-1.11.0 && \
cd googletest && \
mkdir build && \
cd build && \
cmake .. && \
make && \
make install && \
cd ../../ && \
rm -rf googletest

RUN \
cd /opendeck-deps && \
git clone https://github.com/google/glog.git && \
cd glog && \
git checkout v0.6.0 && \
cmake -S . -B build -G "Unix Makefiles" && \
cmake --build build && \
cmake --build build --target install && \
cd ../ && \
rm -rf glog && \
ldconfig

# Disable password prompt for sudo commands
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN echo "alias mkc='make clean'" >> /home/ubuntu/.bashrc
RUN echo "export MAKEFLAGS=-j$(nproc)" >> /home/ubuntu/.bashrc

# Run everything below as $user
USER ubuntu
WORKDIR /home/ubuntu

ADD .devcontainer/.git_branch_bash /home/ubuntu/.git_branch_bash
RUN echo "source ~/.git_branch_bash" >> ~/.bashrc