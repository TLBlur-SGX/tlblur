#!/usr/bin/env bash

# Base values
mkdir -p out
base_cmd="./app out/bench"
num_iter=100000

pws_start=0
pws_end=50
pws_step=5

# Run the command with varying parameters
for pws in $(seq $pws_start $pws_step $pws_end); do
  echo "Running: $base_cmd $num_iter $pws"
  $base_cmd $num_iter $pws
done

