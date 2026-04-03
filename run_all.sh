#!/bin/bash

rm -f results.dat

for instance in instances/*; do
  for pivoting in --first --best; do
    for neighborhood in --transpose --exchange --insert; do
      for start in --random --chenery; do
        ./lop -i "$instance" -p "$pivoting" -n "$neighborhood" -s "$start"
      done
    done
  done
done
