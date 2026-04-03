#!/bin/bash

mkdir -p output
rm -f output/results.dat output/lop-*.dat output/best-known.dat

# Generate best-known.dat in the same instance order (alphabetical)
for instance in instances/*; do
  name=$(basename "$instance")
  grep "^$name " best_known/best_known.txt | awk '{print $2}' >> output/best-known.dat
done

# Run all 12 algorithm combinations on every instance
for instance in instances/*; do
  for pivoting in --first --best; do
    for neighborhood in --transpose --exchange --insert; do
      for start in --random --chenery; do
        ./lop -i "$instance" -p "$pivoting" -n "$neighborhood" -s "$start"
      done
    done
  done
done
