#!/bin/bash

# 安装依赖
git clone git@github.com:AFLplusplus/AFLplusplus.git
git clone git@github.com:thebabush/afl-libprotobuf-mutator.git

sudo apt-get update
sudo apt-get install -y ninja-build build-essential python3-dev automake git flex bison libglib2.0-dev libpixman-1-dev python3-setuptools
# try to install llvm 11 and install the distro default if that fails
# sudo apt-get install -y lld-11 llvm-11 llvm-11-dev clang-11 || sudo apt-get install -y lld llvm llvm-dev clang 
# sudo apt-get install -y gcc-$(gcc --version|head -n1|sed 's/.* //'|sed 's/\..*//')-plugin-dev libstdc++-$(gcc --version|head -n1|sed 's/.* //'|sed 's/\..*//')-dev

# 复制修改的代码至 libprotobuf
cp kp_src/out.proto afl-libprotobuf-mutator/gen/out.proto
cp kp_src/dumper.cc afl-libprotobuf-mutator/src/dumper.cc
cp kp_src/mutator.cc afl-libprotobuf-mutator/src/mutator.cc

# 构建 libprotobuf
pushd afl-libprotobuf-mutator
chmod +x build.sh
./build.sh
make
popd

# 构建 AFL
pushd AFLplusplus
make distrib
popd

# 构建中间程序
gcc -O3 kp_src/target.c -o ./target
