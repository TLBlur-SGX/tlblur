#!/usr/bin/env bash

cat << EOF > meson-clang.ini
[binaries]
c_ld = 'ld'
c = '$PWD/llvm/install/bin/clang'
cpp = '$PWD/llvm/install/bin/clang++'
bolt = '$PWD/llvm/install/bin/llvm-bolt'
llvm-objcopy = '$PWD/llvm/install/bin/llvm-objcopy'
EOF

meson setup -Dsgx_sim=false -Dprefix="$PWD/install" -Dlibdir="lib" --native-file meson-clang.ini build
