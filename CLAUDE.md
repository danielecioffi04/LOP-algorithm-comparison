# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

```bash
make          # compile → produces ./lop binary
make clean    # remove object files and binary
```

Run the solver:
```bash
# Standard iterative improvement (all four flags required)
./lop -i instances/N-tiw56r72_250 -p --first -n --transpose -s --random

# VND (no -n flag needed; neighborhood order is encoded in -p)
./lop -i instances/N-tiw56r72_250 -p --vnd-tei -s --chenery
./lop -i instances/N-tiw56r72_250 -p --vnd-tie -s --chenery

# Run all combinations across all instances (writes to output/)
bash run_all.sh
```

**Flags:**
- `-p`: pivoting rule — `--first`, `--best`, `--vnd-tei` (transpose→exchange→insert), or `--vnd-tie` (transpose→insert→exchange)
- `-n`: neighborhood — `--exchange`, `--insert`, or `--transpose` (not needed for VND)
- `-s`: initial solution — `--random` or `--chenery`

## Architecture

The program solves the **Linear Ordering Problem (LOP)** using local search. A solution is a permutation (`long int *` of length `PSize`) that maximizes the sum of `CostMat[s[i]][s[j]]` for all `i < j`.

**Key globals shared across modules:**
- `CostMat` (`long int **`, defined in `optimization.c`) — the instance's pairwise cost matrix
- `PSize` (`long int`, defined in `instance.c`) — number of elements in the permutation
- `Seed` (`long int`, defined in `utilities.c`) — RNG seed, set deterministically from the sum of all matrix entries

**Module responsibilities:**

| File | Role |
|------|------|
| `src/main.c` | Argument parsing, wires function pointers, runs the local search loop |
| `src/instance.c` | Parses instance files; populates `CostMat` and `PSize` |
| `src/optimization.c` | `computeCost`, initial solution constructors (`createRandomSolution`, `cheneryWatanabe`), pivoting strategies (`firstImprovement`, `bestImprovement`), and move operators (`exchange`, `insert`, `transpose`) |
| `src/utilities.c` | Custom RNG (Numerical Recipes LCG via `ran01`/`randInt`/`generate_random_vector`), matrix allocation |
| `src/timer.c` | Wall/virtual time measurement via `start_timers()` / `elapsed_time()` |

**Pivoting strategies** use function pointers: `pivotingFunction(currentSolution, neighborhoodMoveFunction)`. Both `firstImprovement` and `bestImprovement` accept any move operator as a callback, and detect `transpose` via pointer comparison (`isTransposeMove`) to restrict `j` to `i+1` only. `vnd` always uses `firstImprovement` internally and cycles through 3 neighborhoods (transpose first, then two others), resetting to the first on any improvement.

**Delta computation** (`deltaExchange`, `deltaInsert` in `optimization.c`) computes O(n) incremental cost changes without recomputing the full O(n²) cost after each candidate move.

**Move operators** are self-inverse for `exchange` and `transpose` (applying twice undoes the move). `insert(s, i, j)` shifts elements between positions `i` and `j`.

## Output

Results are written to `output/` (created by `run_all.sh`):
- `output/results.txt` — one row per run: `instance pivoting neighborhood initial cost apd time`
- `output/lop-<pivoting>-<neighborhood>-<initial>.txt` — one cost value per line, one per instance (for statistical analysis)

## Instances and Best Known

- Instance files live in `instances/`, named `N-<name>_<size>` (size is 150 or 250).
- `best_known/best_known.txt` maps instance names to best-known objective values; the program looks up the current instance by name and prints the relative percentage deviation (apd).
- The program must be run from the repository root so that `best_known/best_known.txt` is found at its relative path.
