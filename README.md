# TLBlur

Defense against page fault attacks on Intel SGX using the AEX-Notify hardware extension.
Maintain a Page Access Map (PAM) to inform a page prefetcher and reconstruct the TLB after IRQ in order to hide page accesses from the untrusted operating system.

## Overview

- `llvm`: fork of the LLVM project with backend pass (§6.2) and BOLT pass (§6.3) to add instrumentation
- `runtime`: TLBlur runtime: provides assembly implementation of TLB update instrumentation (§6.1)
- `benchmark`: benchmarking framework (§9.2)
- `enclaves`: set of enclaves to use for profiling and benchmarking (§9.2)
- `sgx-step`: fork of SGX-Step with Rust bindings, profiler tool (§7.2) and attack on libjpeg (§9.1)
- `sgx-sdk`: Intel SGX SDK, with SGX-Step patches and TLBlur prefetcher (§7.1)

## Build instructions

Prerequisites:
- Modern Intel SGX hardware (AEX-Notify support optional and only needed to run prefetcher)
- A Linux distribution supported by Intel SGX

### Cloning the repository

Make sure to clone the repository with all its submodules:

```sh
git clone --recurse-submodules https://github.com/TLBlur-SGX/tlblur.git
```

### LLVM

Prerequisites:
- any modern C/C++ compiler
- `ninja`
- `lld` linker (optional, but strongly recommended)
- `ccache` (optional)

1. Modify `llvm/llvm-configure.sh` to use your installed C compiler, change the number of compiler/link jobs and disable `ccache` and/or `lld` if necessary.
2. Run `llvm/llvm-install.sh` to build and install LLVM

### SGX SDK

SGX-Step provides a script to build and install the Intel SGX SDK on Ubuntu 22.04 at `sgx-step/sdk/intel-sdk/install_SGX_SDK.sh`.
This fork of the SGX SDK includes the TLBlur prefetcher.

Otherwise follow the instructions from the Intel SGX SDK repository and install to `/opt/intel`.

### OpenSSL

To run the OpenSSL benchmarks, OpenSSL for SGX must be compiled with TLBlur instrumentation.

TODO

### Benchmark enclaves

Prerequisites:
- `meson` and `ninja`
- [Intel SGX SDK](#sgx-sdk)
- [LLVM with TLBlur passes](#llvm)
- [OpenSSL](#openssl) (optional)

1. (optional) enable or disable OpenSSL benchmark in `enclaves/meson.build` by (un)commenting the `subdir('openssl')` line
2. Run `./configure-build.sh` to configure the meson build directory
3. Run `ninja -C build install` to build and install enclaves in `$PWD/install`

### Benchmarking tool

Prerequisites:
- Stable Rust toolchain (1.76 or later), see [rustup](https://rustup.rs) for installation instructions.
- [Intel SGX SDK](#sgx-sdk)
- [Benchmark enclaves](#benchmark-enclaves)

1. `export LIBSGXSTEP="$PWD/sgx-step/libsgxstep/`
2. `cd benchmark`
3. `cargo build --release`

### Profiler tool

Prerequisites:
- Stable Rust toolchain (1.76 or later), see [rustup](https://rustup.rs) for installation instructions.
- [Intel SGX SDK](#sgx-sdk)

1. `export LIBSGXSTEP="$PWD/sgx-step/libsgxstep/`
2. `cd sgx-step/app/profiler`
3. `cargo build --release`

### Attack on libjpeg

Prerequisites:
- Stable Rust toolchain (1.76 or later), see [rustup](https://rustup.rs) for installation instructions.
- [Intel SGX SDK](#sgx-sdk)

1. `export LIBSGXSTEP="$PWD/sgx-step/libsgxstep/`
2. `cd sgx-step/app/libjpeg`
3. Run `make all` to build the libjpeg enclave. Note: to ensure reproducibility of the attack, you can skip this step and use the existing binaries
4. `cd attack`
5. `cargo build --release`

## License

- The LLVM project, including TLBlur's instrumentation and binary rewriting passes: Apache License v2.0 with LLVM Exceptions
- Intel SGX for Linux, including TLBlur prefetching code: BSD-3-Clause
- Programs in `enclaves` used for benchmarking: various licenses
- SGX-Step, including attack on libjpeg and profiler tool, and *all* other code in this repo not mentioned: GPLv3
