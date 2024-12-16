FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    linux-headers-5.15.0-75-generic \
    build-essential \
    git \
    libncurses-dev \
    bison \
    flex \
    libssl-dev \
    libelf-dev \
    dwarves \
    binutils-dev \
    bc \
    qemu \
    qemu-kvm \
    libvirt-daemon-system \
    libvirt-clients \
    bridge-utils \
    kmod

RUN apt clean && rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]