FROM debian:10.1-slim

# basic packages
RUN apt-get update && \
    apt-get -y install git python2.7 python-pip locales curl nodejs && \
    curl -sL https://deb.nodesource.com/setup_10.x | bash - && \
    apt-get -y install nodejs build-essential && \
    npm install -g inliner && \
    echo "en_US.UTF-8 UTF-8" > /etc/locale.gen && \
    locale-gen en_US.UTF-8 && \
    update-locale LANG=en_US.UTF-8 && \
    pip install --no-cache-dir -U platformio && \
    apt-get -y autoremove && \
    rm -rf /var/lib/apt/lists/*

# apt-get -y remove python-pip && \
