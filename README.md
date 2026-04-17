## Build

```bash
make        # produces ./lop binary
make clean  # removes object files and binaries
```

Requires GCC.

## Running the solver

The binary must be run from the **repository root**

### Iterative improvement

All four flags are required:

```bash
./lop -i <instance> -p <pivoting> -n <neighborhood> -s <init>
```

| Flag | Option | Description |
|------|--------|-------------|
| `-i` | `instances/<name>` | Path to instance file |
| `-p` | `--first` / `--best` | Pivoting rule: First Improvement or Best Improvement |
| `-n` | `--transpose` / `--exchange` / `--insert` | Neighborhood structure |
| `-s` | `--random` / `--chenery` | Initial solution |

**Example:**
```bash
./lop -i instances/N-tiw56r72_250 -p --first -n --insert -s --chenery
```

### Variable Neighborhood Descent (VND)

The `-n` flag is not used for VND; the neighborhood order is encoded in `-p`:

```bash
./lop -i instances/N-tiw56r72_250 -p --vnd-tei -s --chenery   # T→E→I order
./lop -i instances/N-tiw56r72_250 -p --vnd-tie -s --chenery   # T→I→E order
```

VND always uses First Improvement internally.

## Running all combinations

```bash
bash run_all.sh
```

This runs all 14 configurations (12 iterative improvement + 2 VND) across all 78 instances and writes results to `output/`. Runtime of roughly 20–30 minutes.

## Output

All output is written to `output/` (created by `run_all.sh`):

| File | Contents |
|------|----------|
| `output/results.txt` | One row per run: `instance pivoting neighborhood initial cost apd time` |
| `output/lop-<pivoting>-<neighborhood>-<init>.txt` | One cost value per line, one per instance (in instance order) |
| `output/best-known.txt` | Best-known objective values in instance order |

## Statistical analysis

```bash
Rscript analysis.R
```

Reads all output files and prints:
- Mean APD and total time per algorithm
- All 91 pairwise paired Wilcoxon and t-test results

Requires R.