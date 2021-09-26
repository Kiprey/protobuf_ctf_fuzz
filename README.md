# protobuf_ctf_fuzz

## 一、简介

通过 protobuf + AFLplusplus 进行传统 ctf fuzz。

请参考[这篇博文](https://kiprey.github.io/2021/09/protobuf_ctf_fuzz/) 来了解具体细节。

## 二、构建与运行

构建很简单，只需一行命令即可：

> **网络一定一定一定要好！！！**
>
> 否则还是一条一条的粘贴 ./build.sh 中的命令运行，确保每条命令都成功吧（笑）

```bash
sudo ./build.sh
```

构建好后，将自定义 protobuf 放入 `kp_src/out.proto` 中，同时修改对应的 `kp_src/mutate.cc` 以及 `kp_src/dump.cc`，最后执行以下脚本以更新被修改的部分：

```bash
source ./pre_run.sh
```

> **每次修改**完 `kp_src/` 文件夹下的代码后，或者新开一个终端准备跑 fuzz 前，均需执行`./pre_run.sh`。

之后自己准备 workdir 以及 fuzz_input，然后跑以下命令以启动 fuzz：

> 语料的准备，或许可以修改 `kp_src/dumper.cc` 并借助 `afl-libprotobuf-mutator/dumper` 来生成。

```bash
# 此时工作目录为：protobuf_ctf_fuzz/workdir
../AFLplusplus/afl-fuzz -i ./fuzz_input -o ./fuzz_output -Q -- ../target <CTF_path> @@
```

## 三、例子

根目录下的 `babyheap` 文件作为例子用的 CTF 题目，其 protobuf 描述以及对应的 dumper 和 mutate 代码已经预置于 `kp_src`中。

## 四、可改进的地方

1. libprotobuf-mutator 的变异效果一般，最好手动改进一下
2. 需要实现一下 trim 逻辑，防止样例爆炸
