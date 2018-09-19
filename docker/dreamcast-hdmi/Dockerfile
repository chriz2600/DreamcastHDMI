FROM chriz2600/quartus-lite:latest

RUN apt-get update && apt-get install -y curl make gcc
ADD files/51-usbblaster.rules /etc/udev/rules.d/51-usbblaster.rules
RUN mkdir -p /srv && cd /root && git clone --recurse-submodules https://github.com/chriz2600/DreamcastHDMI.git
ADD files/build /root/build
ADD files/build.projects /root/build.projects
ADD files/program /root/program
ADD files/firmware-packer /root/firmware-packer
ADD files/firmware-unpacker /root/firmware-unpacker
