# TLBlur [![DOI](https://zenodo.org/badge/911608007.svg)](https://doi.org/10.5281/zenodo.14645849)

TLBlur is a *compiler-assisted defense* against page fault attacks on Intel SGX that leverages the AEX-Notify hardware extension.
Instrumented code builds a *Page Access Map (PAM)* at runtime to inform a *page prefetcher* and reconstruct the TLB after IRQ in order to hide page accesses from the untrusted operating system.

## Overview

- `runtime`: TLBlur runtime: provides assembly implementation of TLB update instrumentation (§6.1)
- `llvm`: fork of the LLVM project with backend pass (§6.2) and BOLT pass (§6.3) to add instrumentation
- `sgx-step`: fork of SGX-Step with Rust bindings, profiler tool (§7.2) and attack on libjpeg (§9.1)
  - also contains the Intel SGX SDK as submodule, with SGX-Step patches and TLBlur prefetcher (§7.1)
- `enclaves`: set of enclaves to use for profiling and benchmarking (§9.2)
- `benchmark`: benchmarking framework (§9.2)

## Build instructions

Prerequisites:
- Modern Intel SGX hardware (AEX-Notify support optional and only needed to run prefetcher)
  - See [SGX-hardware](https://github.com/ayeks/SGX-hardware)
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

### SGX-Step

Using SGX-Step requires a kernel module to be built and loaded. Read the instructions in the `sgx-step` repository for more information.

### OpenSSL

To run the OpenSSL benchmarks, OpenSSL for SGX must be compiled with TLBlur instrumentation.

Prerequisites:
- `perl`
- [Intel SGX SDK](#sgx-sdk)
- [LLVM with TLBlur passes](#llvm)

1. `cd enclaves/openssl/intel-sgx-ssl`
2. Download OpenSSL 3.0.13: `wget https://github.com/openssl/openssl/releases/download/openssl-3.0.13/openssl-3.0.13.tar.gz -o Linux/openssl-3.0.13.tar.gz`
3. `cd Linux`
4. `export TLBLUR_LLVM=<path to tlblur repo>/llvm/install`
5. Build and install OpenSSL for SGX: `make clean all`
6. Install with `make install`

### Benchmark enclaves

Prerequisites:
- `meson` and `ninja`
- [Intel SGX SDK](#sgx-sdk)
- [LLVM with TLBlur passes](#llvm)
- [OpenSSL](#openssl) (optional)

1. (optional) enable or disable OpenSSL benchmark in `enclaves/meson.build` by (un)commenting the `subdir('openssl')` line
  - Binary rewriting of OpenSSL can take a very long time (up to 1 hour)
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

## Usage instructions

### Running enclaves with TLBlur prefetcher

1. Enable AEX-Notify in `enclaves/<program>/Enclave/encl.config.xml`
2. Make sure TLBlur prefetching is enabled when entering the enclave by calling `tlblur_enable`
  - example in `ecall_rsa_decode` in `enclaves/rsa/Enclave/encl.c`
3. Run `ninja -C build install` (see [build instructions](#benchmark-enclaves))
4. Run one of the test programs with an enclave binary, e.g. `./install/bin/test-rsa ./install/lib/s-encl-rsa-instrumented-relocs-bolt.so`
  - refer to the [enclave naming scheme](#enclave-naming-scheme) for more information on the enclave binaries
5. IMPORTANT: make sure to disable AEX-Notify again if you continue with running benchmarks or want to use the profiler

### Reproducing benchmark results

1. `cd evaluation`
2. Run the `run_benchmarks.sh` script. This will use the [benchmarking tool](#benchmarking-tool) to produce CSV files with benchmark results.
  - This can take a very long time. To obtain results faster, reduce the number of iterations (at the cost of variance in the results)
3. Use `plot.py` to create the figure and table from the paper (requires pandas, numpy and matplotlib)

### Using the profiler

Example for running the profiler on `libjpeg`, but can be used with any enclave as long as AEX-Notify is **disabled**.

1. Make sure the SGX-Step kernel module is loaded
2. `cd sgx-step/app/libjpeg`
3. `sudo ../profiler/target/release/sgx_tracer --so ./profiler-libjpeg.so -e ./Enclave/encl.so --output trace_libjpeg.vcd`

The resulting VCD file can be analyzed using tools such as [GTKWave](https://gtkwave.github.io/gtkwave/).

### Attacking libjpeg

Change to the `sgx-step/app/libjpeg` directory. Run `cargo run --release -- --help` for usage instructions.

For example, to reconstruct a color image of a Wapiti, assuming the enclave is compiled with AEX-Notify, but without TLBlur prefetching, run:

```sh
cargo run --release -- -o reconstruct_wapiti_aexnotify.bmp -r reconstruct_wapiti_aexnotify.json -i ../img/Wapiti_from_Wagon_Trails.jpg --aexnotify --color enclave -e ../Enclave/encl.so
```

## Additional information

### Enclave naming scheme

Benchmarked enclaves are built with different combinations of instrumentation flags, and can be found in `install/lib`.
The naming scheme can be slightly confusing due to build system limitations, so it is clarified by the following table:

| enclave binary name                                     | compile-time instrumentation | binary instrumentation | instrumentation at jump targets | 16KiB page optimization | additional notes                                   |
|---------------------------------------------------------|------------------------------|------------------------|---------------------------------|-------------------------|----------------------------------------------------|
| s-encl-name.so                                          | no                           | no                     | /                               | /                       |                                                    |
| s-encl-name-relocs-bolt-base.so                         | no                           | no                     | /                               | /                       | rewritten with BOLT, but no binary instrumentation |
| s-encl-name-instrumented.so                             | yes                          | no                     | /                               | /                       |                                                    |
| s-encl-name-instrumented-relocs-bolt-base.so            | yes                          | no                     | /                               | /                       | rewritten with BOLT, but no binary instrumentation |
| s-encl-name-instrumented-relocs-bolt.so                 | yes                          | yes                    | no                              | no                      |                                                    |
| s-encl-name-instrumented-relocs-bolt-16KiB.so           | yes                          | yes                    | no                              | yes                     |                                                    |
| s-encl-name-instrumented-relocs-bolt-jmp-tgts-eflags.so | yes                          | yes                    | yes                             | no                      | store rflags before instrumentation call           |
| s-encl-name-instrumented-relocs-bolt-jmp-tgts.so        | yes                          | yes                    | yes                             | no                      |                                                    |
| s-encl-name-instrumented-relocs-bolt-opt.so             | yes                          | yes                    | yes                             | yes                     |                                                    |
| s-encl-name-relocs-bolt.so                              | no                           | yes                    | no                              | no                      |                                                    |
| s-encl-name-relocs-bolt-16KiB.so                        | no                           | yes                    | no                              | yes                     |                                                    |
| s-encl-name-relocs-bolt-jmp-tgts-eflags.so              | no                           | yes                    | yes                             | no                      | store rflags before instrumentation call           |
| s-encl-name-relocs-bolt-jmp-tgts.so                     | no                           | yes                    | yes                             | no                      |                                                    |
| s-encl-name-relocs-bolt-opt.so                          | no                           | yes                    | yes                             | yes                     |                                                    |


## License

- The LLVM project, including TLBlur's instrumentation and binary rewriting passes: Apache License v2.0 with LLVM Exceptions
- Intel SGX for Linux, including TLBlur prefetching code: BSD-3-Clause
- Programs in `enclaves` used for benchmarking: various licenses
- SGX-Step, including attack on libjpeg and profiler tool, and *all* other code in this repo not mentioned: GPLv3
