#!/bin/bash

mkdir -p output
rm -f output/results.txt output/lop-*.txt

# Run all 12 iterative improvement combinations on every instance
for instance in instances/*; do
  for pivoting in --first --best; do
    for neighborhood in --transpose --exchange --insert; do
      for start in --random --chenery; do
        ./lop -i "$instance" -p "$pivoting" -n "$neighborhood" -s "$start"
      done
    done
  done
done

# Run the 2 VND algorithms (CW initial solution only)
for instance in instances/*; do
  for pivoting in --vnd-tei --vnd-tie; do
    ./lop -i "$instance" -p "$pivoting" -s --chenery
  done
done
