# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty64"

  config.vm.provider "virtualbox" do |vb|
     vb.memory = "1024"
  end

  config.vm.provision "shell", inline: <<-SHELL
      sudo apt-get update
      sudo apt-get install -y build-essential cmake libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libopencv-dev libboost-program-options-dev

      # Install FFMpeg for H.265 support
      # Pre-reqs
      sudo apt update -qq
      sudo apt -y install libgles2-mesa-dev
      sudo apt-get -y install \
        autoconf \
        automake \
        build-essential \
        cmake \
        git-core \
        libass-dev \
        libfreetype6-dev \
        libsdl2-dev \
        libtool \
        libva-dev \
        libvdpau-dev \
        libvorbis-dev \
        libxcb1-dev \
        libxcb-shm0-dev \
        libxcb-xfixes0-dev \
        pkg-config \
        texinfo \
        wget \
        zlib1g-dev
      mkdir -p ~/ffmpeg_sources ~/bin

      # Modules
      # NASM & YASM
      sudo apt install nasm yasm
      # libx265
      sudo apt-get install mercurial libnuma-dev && \
      printf "[ui]\ntls = False" >> ~/.hgrc && \
      cd ~/ffmpeg_sources && \
      if cd x265 2> /dev/null; then hg pull && hg update; else hg clone https://bitbucket.org/multicoreware/x265; fi && \
      cd x265/build/linux && \
      PATH="$HOME/bin:$PATH" cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$HOME/ffmpeg_build" -DENABLE_SHARED=off ../../source && \
      PATH="$HOME/bin:$PATH" make && \
      make install

      # Install FFmpeg
      cd ~/ffmpeg_sources && \
      wget -O ffmpeg-snapshot.tar.bz2 https://ffmpeg.org/releases/ffmpeg-snapshot.tar.bz2 && \
      tar xjvf ffmpeg-snapshot.tar.bz2 && \
      cd ffmpeg && \
      PATH="$HOME/bin:$PATH" PKG_CONFIG_PATH="$HOME/ffmpeg_build/lib/pkgconfig" ./configure \
        --prefix="$HOME/ffmpeg_build" \
        --pkg-config-flags="--static" \
        --extra-cflags="-I$HOME/ffmpeg_build/include" \
        --extra-ldflags="-L$HOME/ffmpeg_build/lib" \
        --extra-libs="-lpthread -lm" \
        --bindir="$HOME/bin" \
        --enable-gpl \
        --enable-libass \
        --enable-libfreetype \
        --enable-libx265 \
        --enable-nonfree && \
      PATH="$HOME/bin:$PATH" make && \
      make install && \
      hash -r
      sudo su -s /bin/bash -c "printf \"\n/home/vagrant/ffmpeg_build/lib\n\" >> /etc/ld.so.conf"
      sudo ldconfig
  SHELL
end
