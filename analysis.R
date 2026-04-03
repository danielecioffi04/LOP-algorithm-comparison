# Exercise 1.1 — Statistical Analysis

# Read best known solution values (one per line, same order as instances)
best.known <- read.table("output/best-known.dat")$V1

# Helper: load costs for one algorithm and compute RPD
load.rpd <- function(filename) {
  cost <- read.table(filename)$V1
  100 * (best.known - cost) / best.known
}

# Load RPD for all 12 algorithm combinations
fi.tr.rand  <- load.rpd("output/lop-first-transpose-random.dat")
fi.tr.cw    <- load.rpd("output/lop-first-transpose-chenery.dat")
fi.ex.rand  <- load.rpd("output/lop-first-exchange-random.dat")
fi.ex.cw    <- load.rpd("output/lop-first-exchange-chenery.dat")
fi.ins.rand <- load.rpd("output/lop-first-insert-random.dat")
fi.ins.cw   <- load.rpd("output/lop-first-insert-chenery.dat")
be.tr.rand  <- load.rpd("output/lop-best-transpose-random.dat")
be.tr.cw    <- load.rpd("output/lop-best-transpose-chenery.dat")
be.ex.rand  <- load.rpd("output/lop-best-exchange-random.dat")
be.ex.cw    <- load.rpd("output/lop-best-exchange-chenery.dat")
be.ins.rand <- load.rpd("output/lop-best-insert-random.dat")
be.ins.cw   <- load.rpd("output/lop-best-insert-chenery.dat")

algorithms <- list(
  "first_transpose_random"  = fi.tr.rand,
  "first_transpose_chenery" = fi.tr.cw,
  "first_exchange_random"   = fi.ex.rand,
  "first_exchange_chenery"  = fi.ex.cw,
  "first_insert_random"     = fi.ins.rand,
  "first_insert_chenery"    = fi.ins.cw,
  "best_transpose_random"   = be.tr.rand,
  "best_transpose_chenery"  = be.tr.cw,
  "best_exchange_random"    = be.ex.rand,
  "best_exchange_chenery"   = be.ex.cw,
  "best_insert_random"      = be.ins.rand,
  "best_insert_chenery"     = be.ins.cw
)

# ── Summary statistics ─────────────────────────────────────────────────────────

# Read total times from results.dat
data <- read.table("output/results.dat", header = TRUE)
data$algorithm <- paste(data$pivoting, data$neighborhood, data$initial, sep = "_")

cat("=== Average RPD and Total Time per Algorithm ===\n\n")
for (name in names(algorithms)) {
  avg.rpd    <- mean(algorithms[[name]])
  total.time <- sum(data$time[data$algorithm == name])
  cat(sprintf("%-25s  avg_rpd = %.4f%%  total_time = %.4fs\n", name, avg.rpd, total.time))
}

# ── Pairwise statistical tests ─────────────────────────────────────────────────

cat("\n=== Pairwise Statistical Tests ===\n\n")

alg.names <- names(algorithms)
for (i in 1:(length(alg.names) - 1)) {
  for (j in (i + 1):length(alg.names)) {
    a <- algorithms[[alg.names[i]]]
    b <- algorithms[[alg.names[j]]]

    # Paired t-test and Wilcoxon test (paired because same instances used)
    t.p <- t.test(a, b, paired = TRUE)$p.value
    w.p <- wilcox.test(a, b, paired = TRUE, exact = FALSE)$p.value

    cat(sprintf("%s vs %s\n", alg.names[i], alg.names[j]))
    cat(sprintf("  t.test:  p = %.4f  significant: %s\n", t.p, ifelse(t.p < 0.05, "YES", "NO")))
    cat(sprintf("  wilcox:  p = %.4f  significant: %s\n\n", w.p, ifelse(w.p < 0.05, "YES", "NO")))
  }
}
