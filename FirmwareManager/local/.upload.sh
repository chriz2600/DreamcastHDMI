#!/bin/bash

username=Test
password=testtest

function doUpload() {
    target=$1
    file=$2
    curl --digest --user ${username}:${password} \
        -o /dev/null -F "file=@${file}" \
        "http://dc-firmware-manager.local/upload/${target}"
}

function doFlash() {
    curl --digest --user ${username}:${password} \
        "http://dc-firmware-manager.local/flash/${target}" && echo "OK" || echo "Error"
}


