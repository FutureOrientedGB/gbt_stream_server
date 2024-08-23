#!/bin/bash

if [ ! -d "vcpkg" ]; then
    git clone https://github.com/microsoft/vcpkg.git
fi

cd vcpkg || exit

git reset --hard fb544875b93bffebe96c6f720000003234cfba08

if [ ! -f "vcpkg" ]; then
    ./bootstrap-vcpkg.sh
fi

