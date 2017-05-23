FROM chriz2600/quartus-lite

ADD files/51-usbblaster.rules /etc/udev/rules.d/51-usbblaster.rules
RUN mkdir -p /srv && cd /root && git clone https://github.com/chriz2600/DreamcastHDMI.git
ADD files/build /root/build
ADD files/build.projects /root/build.projects
ADD files/program /root/program
RUN mkdir -p /root/JIC/
ADD files/JIC.tgz /root/JIC/

