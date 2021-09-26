#! /bin/bash
# 复制修改的代码至 libprotobuf
cp kp_src/out.proto afl-libprotobuf-mutator/gen/out.proto
cp kp_src/dumper.cc afl-libprotobuf-mutator/src/dumper.cc
cp kp_src/mutator.cc afl-libprotobuf-mutator/src/mutator.cc

# 由于已经构建好了，因此只需 make 即可
pushd afl-libprotobuf-mutator
make
popd

# 配置环境变量
export AFL_CUSTOM_MUTATOR_ONLY=1
export AFL_CUSTOM_MUTATOR_LIBRARY=$(pwd)/afl-libprotobuf-mutator/libmutator.so
export AFL_USE_QASAN=1

# 输出信息
echo '=========================================================================================================='
echo 'Just prepare fuzz_input and run the command to start fuzz:                                                '
echo '  AFLplusplus/afl-fuzz -i workdir/fuzz_input -o workdir/fuzz_output -Q -- ./target <target_binary_path> @@'
echo '                                                                                                          '
echo 'e.g.  AFLplusplus/afl-fuzz -i workdir/fuzz_input -o workdir/fuzz_output -Q -- ./target ./babyheap @@      '
echo '=========================================================================================================='