#!/usr/bin/env bash

set -e

TLBLUR_DIR=$(pwd)

echo "--- [ building LLVM ] ---"
pushd llvm
  ./llvm-configure.sh
  ./llvm-install.sh
popd

echo "--- [ building Intel SGX SDK ] ---"
pushd sgx-step/sdk/intel-sdk
  ./install_SGX_SDK.sh
popd

echo "--- [ building OpenSSL ] ---"
pushd enclaves/openssl/intel-sgx-ssl
  wget https://github.com/openssl/openssl/releases/download/openssl-3.0.13/openssl-3.0.13.tar.gz -o Linux/openssl-3.0.13.tar.gz
  pushd Linux
    export TLBLUR_LLVM=$TLBLUR_DIR/llvm/install
    make clean all
    make install
  popd
popd

echo "--- [ building benchmark enclaves ] ---"
./configure_build.sh
ninja -C build install

echo "--- [ building tools ] ---"
export LIBSGXSTEP="$TLBLUR_DIR/sgx-step/libsgxstep/"

pushd benchmark
  cargo build --release
popd

pushd sgx-step/app/profiler
  cargo build --release
popd

echo "--- [ building libjpeg attack ] ---"
pushd sgx-step/app/libjpeg
  make all
  pushd attack
    cargo build --release
  popd
popd


echo "****************************************************************************"
echo "*                             -@@@@@@@@@@@@@@@-                            *"
echo "*                                   .@@@@@@   :@@@%              @#        *"
echo "*                   #@@@@@@@@@@@@@@@@*@@@   :   @@@@@=         %@@@        *"
echo "*                    .@@@@@@@@@@@@@@@@@#  =@@@   @@@@@@.     @@@@@@        *"
echo "*                                @@@@@@   @@@@@  .@@@@@@: *@@@@@@@@        *"
echo "*                  .-=*#%%@@@@@@@%%@@@@  @@@@@@   @@@@@@@@@@@@@@@@:        *"
echo "*                @@@@@@@@@@@@@@@@@@@@@%  @@@@@@  :@@@@@@@@@@@@@@@@         *"
echo "*                                        *@@@@   @@@@@@@@@@@@@@@@          *"
echo "*                                         @@@.   .@@@@*@@@@@@@@.           *"
echo "*     #@@@    @@@@@@@@@@@@@@@@@@@@@@       @.    @@@   @@@@@@@             *"
echo "*      *#.    :@@@@@@@@@@@@@@@@@@@@.            @@-  @@@-@@@@              *"
echo "*                                     :@:      @@#  @@@    @@@             *"
echo "*                                     @@      .@@   @@  *@. *@@.           *"
echo "*        %@@@   *@@@@@@@@@@%   @@@   @@@      =@@   @@  .@@  .@@#          *"
echo "*                             .@@.   @@:            @@*  %@@  :@@          *"
echo "*                             @@@   %@@             *@@:  @@@  @@.         *"
echo "* .:-.      ::::::::::::::    @@@   @@@   #   @@@@%  -@@:  @@. @@.         *"
echo "* @@@@=    @@@@@@@@@@@@@@@@   @@@   @@+   @@   .@@@#  .@@-  @@  @:         *"
echo "*                             @@@   @@   +@@            @@- .@@ @@         *"
echo "*                             .@@:  @@   @@%             @@+ =@- @         *"
echo "*   @@@@@@@@@    @@@@@@@@@@    @@@  %@   @@    +@@@@@@+   @@# *@..@        *"
echo "*   @@@@@@@@@    @@@@@@@@@@@   -@@@  @  :@@   @@@@@@@@@@@  +@@ @@ +-       *"
echo "*                       =@@@%   @@@@@@  :@@  .@@%       @@@ .@@ @@ @       *"
echo "*                        %@@@-   @@@@@@  @@  +@@    :-.   @@  @@ @@ @      *"
echo "*    @@@=   @@@@@@@@@@+   #@@@=   *@@@@@ @@  +@@  %@@@@@@@  @=-@@@@@@@     *"
echo "*     @.     @@@@@@@@@     #@@@@    @@@@@@@  .@@ .@@@   %@@@ @@@@@@@@@@    *"
echo "*                           :@@@@.   =@@@@@@  @@*@@@       +@@@@@@@@@@@@   *"
echo "*                             @@@@@    #@@@@@ .@@@@= -@@@@+ @@@@@@@@@@@@*  *"
echo "* @@@@@: =@@@@@@@@@@@@@@@@@.   +@@@@#     @@@+   -@  @@@@@@@@@@@#@@@@@@@@  *"
echo "*    ..    .+@@@@@@@@@@@@@@@=    @@@@@@              *@   @@@@@@  -@@@@@   *"
echo "*                            @     @@@@@@               .@@@@@@@@@@@@@@@:  *"
echo "*                           @@@@     @@@@@@@.            @@@@@@@@@@@@@@@   *"
echo "*        @@@@@@@:   %@@@@=   @@@@@     .@@@@.            @@@@@@@@@@@@@*    *"
echo "*                              @@@@@+     -@               @@@@@@@@@@      *"
echo "*                                @@@@@@.                      +%#-         *"
echo "*                                  *@@@@@@                                 *"
echo "*               @@@@.   @@@@@@@@     .@@@@@-                               *"
echo "*                                       #@@                                *"                                                                       
echo "****************************************************************************"
echo "******************* TLBlur: Build completed successfully *******************"
echo "****************************************************************************"
