# Exercise 1.1 — Statistical Analysis

# Read best known solution values (one per line, same order as instances)
best.known <- read.table("output/best-known.txt")$V1

# Helper: load costs for one algorithm and compute RPD
load.rpd <- function(filename) {
  cost <- read.table(filename)$V1
  100 * (best.known - cost) / best.known
}

# Load RPD for all 12 iterative improvement combinations + 2 VND algorithms
fi.tr.rand  <- load.rpd("output/lop-first-transpose-random.txt")
fi.tr.cw    <- load.rpd("output/lop-first-transpose-chenery.txt")
fi.ex.rand  <- load.rpd("output/lop-first-exchange-random.txt")
fi.ex.cw    <- load.rpd("output/lop-first-exchange-chenery.txt")
fi.ins.rand <- load.rpd("output/lop-first-insert-random.txt")
fi.ins.cw   <- load.rpd("output/lop-first-insert-chenery.txt")
be.tr.rand  <- load.rpd("output/lop-best-transpose-random.txt")
be.tr.cw    <- load.rpd("output/lop-best-transpose-chenery.txt")
be.ex.rand  <- load.rpd("output/lop-best-exchange-random.txt")
be.ex.cw    <- load.rpd("output/lop-best-exchange-chenery.txt")
be.ins.rand <- load.rpd("output/lop-best-insert-random.txt")
be.ins.cw   <- load.rpd("output/lop-best-insert-chenery.txt")
vnd.tei.cw  <- load.rpd("output/lop-vnd-tei-chenery.txt")
vnd.tie.cw  <- load.rpd("output/lop-vnd-tie-chenery.txt")

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
  "best_insert_chenery"     = be.ins.cw,
  "vnd_tei_chenery"         = vnd.tei.cw,
  "vnd_tie_chenery"         = vnd.tie.cw
)

# ── Summary statistics ─────────────────────────────────────────────────────────

# Read total times from results.txt
data <- read.table("output/results.txt", header = TRUE)
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
