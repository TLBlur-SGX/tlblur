# Development setup

## Building

### Building LLVM

See https://llvm.org/docs/GettingStarted.html.

You can also use the scripts in the `llvm` subdirectory to configure and run `cmake` in order to compile LLVM.

### Installing SGX SDK

Follow SGX-Step instructions: https://github.com/jovanbulck/sgx-step/

Also make sure to build and load the sgx-step kernel module if you want to use the profiler.

### Building examples

Build the examples with `meson` and `ninja`:

```sh
./configure_build.sh
meson compile -C build
```

Make sure the LLVM install directory is part of your `$PATH`, or update `meson-clang.ini`by setting the right paths for `clang`.

### Building the profiler

The building of the profiler (and benchmarking tool) is a bit hacky, as we use 2 independent build systems, `meson` and `cargo` for C and Rust respectively.

First make sure you have built everything with `meson` (see previous section).
To build the profiler you need a relatively recent Rust toolchain, latest stable version is recommended.
First run `export LIBSGXSTEP="$PWD/sgx-step/libsgxstep/"`, then build the profiler with `cargo` (e.g. run `cargo build --release` in the `profiler` subdirectory).

### Building the benchmarking tool

Same as building the profiler, see `benchmark` subdirectory.

## Running enclaves

All binaries are installed to `install/bin`, and shared libraries are installed to `install/lib`. `s-encl-*.so` files are signed enclave binaries and `libprof-*.so` are profiler plugins that load and call the respective enclave when running the profiler with these libraries. Similarly, `libbench-*.so` are benchmark plugins.

### Profiling enclaves

See the command line help for the profiler for more information. Example usage:

```bash
sudo ./profiler --so libprof-libjpeg.so -e s-encl-libjpeg-instrumented-relocs-bolt.so --size 20 --trace --defense -o trace_libjpeg_defense.vcd
```

Note that you currently need 2 `.so` files, one is the (trusted) enclave binary, the other one is a (untrusted) "bridge" library that does the actual ecalls into the enclave.

Use GTKWave to analyze the VCD file. Conversion to `fst` using `vcd2fst` is recommended for large VCD files.

### Benchmarking enclaves

Similar to the profiler, example usage:

```bash
./benchmark --so libbench-libjpeg.so -e s-encl-libjpeg-instrumented-relocs-bolt.so --iterations 1000 --args img/birds-gray.jpg 50000 2000000
```

TODO: generate output file and process data

## Development

### Creating new enclaves to profile or benchmark

The following steps must be taken to add a new enclave to profile or benchmark.

1. To add a new enclave (e.g. `myenclave`), create a new subdirectory in `enclaves`. Easiest would be to copy one of the existing ones like `rsa` or `libjpeg`.
Also make sure to add a new `subdir()` call in `enclaves/meson.build` to add the enclave to the build system.
2. Add a file called `myenclave.c` in `profiler/lib`. This file should implement `profiler_setup` and `profiler_run` and is used by the profiler to call the enclave.
3. Add a file called `myenclave.c` in `benchmark/lib`. This file should implement `tlblur_benchmark_setup`, which is called before running the benchmark, e.g. to load a JPEG image into the enclave.
4. Make sure your enclave implements the `benchmark_inner` method. This is called by another enclave function called `ecall_run_benchmark` which is linked into each enclave. The `benchmark_inner` method takes a benchmark id as input, which is a parameter that is used to select between different benchmarks in the same enclave, but can otherwise be interpreted freely.



