#!/bin/bash

username="dchdmi"
password="testtest"
hostname=${ESP_HOSTNAME:-dc-firmware-manager.local}

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


