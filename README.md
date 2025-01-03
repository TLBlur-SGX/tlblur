# TLBlur

Defense against page fault attacks on Intel SGX using the AEX-Notify hardware extension.
Maintain a Page Access Map (PAM) to inform a page prefetcher and reconstruct the TLB after IRQ in order to hide page accesses from the untrusted operating system.

## Development setup

See [setup](./setup.md).

## Notes

See [notes](./notes/README.md) for a collection of thoughts and ideas. Feel free to add anything that comes to mind.

## Subdirectories

- `llvm`: fork of the LLVM project with passes to add instrumentation
- `runtime`: TLBlur runtime: provides assembly implementation of TLB update instrumentation (mainly used for code page instrumentation)
- `benchmark`: benchmarking framework
- `enclaves`: set of enclaves to use for profiling and benchmarking
- `sgx-step`: fork of SGX-Step with some minor additions and Rust bindings
- `sgx-sdk`: Intel SGX SDK, with SGX-Step patches and TLBlur prefetcher

All other directories are prototypes and experiments that may or may not be relevant.

## License

- The LLVM project, including TLBlur's instrumentation and binary rewriting passes: Apache License v2.0 with LLVM Exceptions
- Intel SGX for Linux, including TLBlur prefetching code: BSD-3-Clause
- Programs in `enclaves` used for benchmarking: various licenses
- SGX-Step, including attack on libjpeg and profiler tool, and *all* other code in this repo not mentioned: GPLv3
