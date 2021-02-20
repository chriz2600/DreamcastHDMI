#!/bin/bash

cd $(dirname $0)

function running_in_docker() {
    test -d /proc && awk -F/ '$2 == "docker"' /proc/self/cgroup | read
}

function docker_run {
    if running_in_docker ; then
        ./build-all "$@"
    else
        docker run --rm -it \
            -v $(pwd)/..:/build \
            registry.gitlab.com/chriz2600/platformio:5.0.1 \
            /build/ESP/pio.sh "$@"
    fi
}

docker_run "$@"
