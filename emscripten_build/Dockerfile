FROM ubuntu:24.04

RUN if [ ! -e /etc/timezone ]; then \ 
    echo "Etc/UTC" > /etc/timezone && \ 
    ln -s /usr/share/zoneinfo/Etc/UTC /etc/localtime && \ 
    apt-get update -q -qq -y && \ 
    apt-get install -q -y --no-install-recommends tzdata && \ 
    rm -rf /var/lib/apt/lists/* ; \ 
    fi

RUN apt-get update -q -qq && apt-get install -q -y --no-install-recommends \
    dirmngr \
    gnupg2 ca-certificates lsb-release openssl curl wget aptitude \
    cmake git make build-essential ninja-build \
    python3-dev libclang-dev libgl-dev \
    sudo \
    > /dev/null \
    && rm -rf /var/lib/apt/lists/*

## delete ubuntu
RUN touch /var/mail/ubuntu && chown ubuntu /var/mail/ubuntu && userdel -r ubuntu

ARG USERNAME=luser
ARG GROUPNAME=luser
ARG UID=1000
ARG GID=1000
ARG PASSWORD=luser

RUN groupadd -g $GID $GROUPNAME && \
    useradd -m -s /bin/bash -u $UID -g $GID -G sudo $USERNAME && \
    echo $USERNAME:$PASSWORD | chpasswd && \
    echo "$USERNAME   ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

USER $USERNAME
WORKDIR /home/$USERNAME/
