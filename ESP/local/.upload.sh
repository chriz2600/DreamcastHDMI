#!/bin/bash

username="please"
password="installme!"
hostname=${ESP_HOSTNAME:-192.168.4.1}

function doUpload() {
    target=$1
    file=$2
    curl --digest --user ${username}:${password} \
        -o /dev/null -F "file=@${file}" \
        "http://${hostname}/upload/${target}"
}

function doFlash() {
    curl --digest --user ${username}:${password} \
        "http://${hostname}/flash/${target}" && echo "OK" || echo "Error"
}


