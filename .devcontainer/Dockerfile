FROM ubuntu:20.04

ARG user=opendeck
ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll"

RUN \
apt-get update && \
DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y \
make \
srecord \
git \
git-lfs \
wget \
gdb \
imagemagick \
dfu-util \
xz-utils \
gcc \
g++ \
bsdmainutils \
clang \
ca-certificates \
sudo \
bash-completion \
ccache \
unzip \
cmake \
lsb-release \
software-properties-common \
python3-pip \
gcc-multilib \
gpg-agent \
&& rm -rf /var/lib/apt/lists/*

RUN \
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)" && \
apt-get install -y --no-install-recommends \
clang-format-14 \
clang-tidy-14

RUN \
mkdir /opendeck-deps

RUN \
cd /opendeck-deps && \
wget ${WGET_ARGS} https://github.com/TomWright/dasel/releases/download/v1.22.1/dasel_linux_amd64 && \
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
cd /opt && \
wget ${WGET_ARGS} https://github.com/facebook/infer/releases/download/v1.1.0/infer-linux64-v1.1.0.tar.xz && \
tar -xf infer-linux64-v1.1.0.tar.xz && \
rm infer-linux64-v1.1.0.tar.xz && \
cp /lib/clang/14.0.5/lib/linux/libclang_rt.profile-x86_64.a /opt/infer-linux64-v1.1.0/lib/infer/facebook-clang-plugins/clang/install/lib/clang/11.1.0/lib/linux

ENV PATH="/opt/infer-linux64-v1.1.0/bin:/opendeck-deps/arduino-1.8.19/hardware/tools/avr/bin:/opendeck-deps/gcc-arm-none-eabi-10.3-2021.10/bin:$PATH"

#setup gtest/gmock
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

#use newer version of glog
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

RUN \
pip install compiledb

#don't run as root!
RUN adduser --disabled-password --gecos '' $user

#add user to sudo group
RUN adduser $user sudo

#disable password prompt for sudo commands
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN echo "alias mkc='make clean'" >> /home/$user/.bashrc
RUN echo "export MAKEFLAGS=-j$(nproc)" >> /home/$user/.bashrc

#run everything below as $user
USER $user
WORKDIR /home/$user

RUN \
sudo update-alternatives --install /usr/local/bin/clang-format clang-format /usr/bin/clang-format-14 20 && \
sudo update-alternatives --install /usr/local/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-14 20 && \
sudo update-alternatives --install /usr/local/bin/run-clang-tidy run-clang-tidy /usr/bin/run-clang-tidy-14 20