#!/bin/bash

mkdir -p output
rm -f output/results.txt output/lop-*.txt output/best-known.txt

# Generate best-known.txt in the same instance order (alphabetical)
for instance in instances/*; do
  name=$(basename "$instance")
  grep "^$name " best_known/best_known.txt | awk '{print $2}' >> output/best-known.txt
done

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
