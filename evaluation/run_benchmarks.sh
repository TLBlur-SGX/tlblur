#!/usr/bin/env bash

PROJECT_ROOT=$(realpath "..")
LIB="$PROJECT_ROOT/install/lib"
OUT_DIR="out"

mkdir -p "$OUT_DIR"

run() {
  "$PROJECT_ROOT/benchmark/target/release/benchmark" $@
}

run_benchmark() {
  OUTPUT="$OUT_DIR/$2-$1$3.csv"
  echo "$OUTPUT"

  if [ ! -f "$OUTPUT" ]; then
    run --so "$LIB/libbench-$2.so" -e "$LIB/s-encl-$2$3.so" \
        --iterations $ITERATIONS --warmup $WARMUP \
        -o $OUTPUT --args "${@:4}"
  else
    echo "already exists, skipping..."
  fi
}

run_all_variants() {
  run_benchmark $1 $2 "" "${@:3}"
  run_benchmark $1 $2 "-relocs-bolt-base" "${@:3}"
  run_benchmark $1 $2 "-relocs-bolt" "${@:3}"
  run_benchmark $1 $2 "-relocs-bolt-16KiB" "${@:3}"
  run_benchmark $1 $2 "-relocs-bolt-jmp-tgts-eflags" "${@:3}"
  run_benchmark $1 $2 "-relocs-bolt-jmp-tgts" "${@:3}"
  run_benchmark $1 $2 "-relocs-bolt-opt" "${@:3}"
  run_benchmark $1 $2 "-instrumented" "${@:3}"
  run_benchmark $1 $2 "-instrumented-relocs-bolt-base" "${@:3}"
  run_benchmark $1 $2 "-instrumented-relocs-bolt" "${@:3}"
  run_benchmark $1 $2 "-instrumented-relocs-bolt-16KiB" "${@:3}"
  run_benchmark $1 $2 "-instrumented-relocs-bolt-jmp-tgts-eflags" "${@:3}"
  run_benchmark $1 $2 "-instrumented-relocs-bolt-jmp-tgts" "${@:3}"
  run_benchmark $1 $2 "-instrumented-relocs-bolt-opt" "${@:3}"
}

WARMUP=10
ITERATIONS=200
run_all_variants "pbkdf2" "wolfssl" 0
run_all_variants "aes-cbc" "wolfssl" 1

WARMUP=20000
ITERATIONS=2000000
run_all_variants "aes-gcm" "openssl" 0

WARMUP=1000
ITERATIONS=20000
run_all_variants "rsa" "openssl" 1

WARMUP=100
ITERATIONS=2000
run_all_variants "yescrypt" "yescrypt" 0
# run_all_variants "scrypt" "yescrypt" 1
run_all_variants "pbkdf2-sha256" "yescrypt" 2

WARMUP=10000
ITERATIONS=200000
run_all_variants "stressmem" "micro" 1
run_all_variants "fibonacci" "micro" 2

WARMUP=100000
ITERATIONS=2000000
run_all_variants "long-func" "micro" 3

WARMUP=100
ITERATIONS=2000
run_all_variants "long-loop" "micro" 4

WARMUP=100
ITERATIONS=2000
run_all_variants "birds-gray" "libjpeg" img/birds-gray.jpg 10000000 10000000
