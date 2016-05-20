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
  SHELL
end
