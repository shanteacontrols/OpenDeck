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
&& rm -rf /var/lib/apt/lists/*

RUN \
mkdir /opendeck-deps

RUN \
cd /opendeck-deps && \
wget ${WGET_ARGS} https://github.com/TomWright/dasel/releases/download/v1.15.0/dasel_linux_amd64 && \
chmod +x dasel_linux_amd64 && \
mv $(pwd)/dasel_linux_amd64 /usr/local/bin/dasel

RUN \
cd /opendeck-deps && \
wget ${WGET_ARGS} https://downloads.arduino.cc/arduino-1.8.13-linux64.tar.xz -O arduino.tar.xz && \
tar -xf arduino.tar.xz && \
rm arduino.tar.xz && \
cd arduino-1.8.13 && \
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
wget ${WGET_ARGS} https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 && \
tar -xf gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 && \
rm gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2

ENV PATH="/opendeck-deps/arduino-1.8.13/hardware/tools/avr/bin:/opendeck-deps/gcc-arm-none-eabi-10-2020-q4-major/bin:$PATH"
ENV MAKEFLAGS="-j8"

#don't run as root!
RUN adduser --disabled-password --gecos '' $user

#add user to sudo group
RUN adduser $user sudo

#disable password prompt for sudo commands
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN echo "alias mkc='make clean'" >> /home/$user/.bashrc

#run everything below as $user
USER $user
WORKDIR /home/$user