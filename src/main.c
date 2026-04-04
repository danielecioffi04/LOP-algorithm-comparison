/*  Heuristic Optimization assignment, 2015.
    Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
    of Tommaso Schiavinotto:
    ---
    ILSLOP Iterated Local Search Algorithm for Linear Ordering Problem
    Copyright (C) 2004  Tommaso Schiavinotto (tommaso.schiavinotto@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "instance.h"
#include "utilities.h"
#include "timer.h"
#include "optimization.h"

char *FileName;
char *instanceName;  /* bare instance name, without path */
char *pivotingRule; //--first or --best
char *neighborhoodType; //--exchange, --insert, --transpose,
char *initialSolutionType; //--random, --chenery
long int bestKnown;

long long int (*pivotingFunction)(long int *, void (*move)(long int *, int, int));
void (*neighborhoodMoveFunction)(long int *, int, int);
void (*initialSolutionFunction)(long int *);


void readOpts(int argc, char **argv) {
  char opt;
  const char *inputInstanceName;
  const char *lastSlash;

  FileName = NULL;
  pivotingRule = NULL;
  neighborhoodType = NULL;
  initialSolutionType = NULL;
  bestKnown = 0;

  while ((opt = getopt(argc, argv, "i:p:n:s:")) != -1) {
    switch (opt) {
      case 'i':
        FileName = optarg;
        break;
      case 'p':
        pivotingRule = optarg;
        break;
      case 'n':
        neighborhoodType = optarg;
        break;
      case 's':
        initialSolutionType = optarg;
        break;
      default:
        printf("Usage: %s -i <instance_file> -p <pivoting_rule> [-n <neighborhood_type>] -s <initial_solution_type>\n", argv[0]);
        printf("  pivoting_rule: --first, --best, --vnd-tei, --vnd-tie\n");
        printf("  neighborhood_type: --exchange, --insert, --transpose (not needed for VND)\n");
        printf("  initial_solution_type: --random, --chenery\n");
        exit(1);
    }
  }

  /* VND: internally split --vnd-tei/--vnd-tie into pivoting=vnd and neighborhood=tei/tie */
  if (pivotingRule != NULL && strcmp(pivotingRule, "--vnd-tei") == 0) {
    pivotingRule = "--vnd";
    neighborhoodType = "--tei";
  } else if (pivotingRule != NULL && strcmp(pivotingRule, "--vnd-tie") == 0) {
    pivotingRule = "--vnd";
    neighborhoodType = "--tie";
  }

  int isVND = (pivotingRule != NULL && strcmp(pivotingRule, "--vnd") == 0);

  if (FileName == NULL || pivotingRule == NULL || initialSolutionType == NULL ||
      (!isVND && neighborhoodType == NULL)) {
    printf("Usage: %s -i <instance_file> -p <pivoting_rule> [-n <neighborhood_type>] -s <initial_solution_type>\n", argv[0]);
    printf("  pivoting_rule: --first, --best, --vnd-tei, --vnd-tie\n");
    printf("  neighborhood_type: --exchange, --insert, --transpose (not needed for VND)\n");
    printf("  initial_solution_type: --random, --chenery\n");
    exit(1);
  }

  inputInstanceName = FileName;
  lastSlash = strrchr(FileName, '/');
  if (lastSlash != NULL && *(lastSlash + 1) != '\0') {
    inputInstanceName = lastSlash + 1;
  }
  instanceName = (char *)inputInstanceName;

  /* retrieve bestKnown from best_known/best_known.txt*/
  FILE *f = fopen("best_known/best_known.txt", "r");
  if (f) {
    char line[256];
    while (fgets(line, sizeof(line), f)) {
      char instanceName[256];
      long int instanceBestKnown;
      if (sscanf(line, "%s %ld", instanceName, &instanceBestKnown) == 2) {
        if (strcmp(instanceName, inputInstanceName) == 0) {
          bestKnown = instanceBestKnown;
          break;
        }
      }
    }
    fclose(f);
  }
}



int main (int argc, char **argv) 
{
  long int i,j;
  int iterations;
  long int *currentSolution;
  long long int cost, initialCost, candidateCost;

  /* Do not buffer output */
  setbuf(stdout,NULL);
  setbuf(stderr,NULL);
  
  if (argc < 2) {
    printf("No instance file provided (use -i <instance_name>). Exiting.\n");
    exit(1);
  }
  
  /* Read parameters */
  readOpts(argc, argv);

  /* Read instance file */
  CostMat = readInstance(FileName);
  printf("Data have been read from instance file. Size of instance = %ld.\n\n", PSize);

  /* initialize random number generator, deterministically based on instance.
   * To do this we simply set the seed to the sum of elements in the matrix, so it is constant per-instance,
   but (most likely) varies between instances */
  Seed = (long int) 0;
    for (i=0; i < PSize; ++i)
      for (j=0; j < PSize; ++j)
        Seed += (long int) CostMat[i][j];
  printf("Seed used to initialize RNG: %ld.\n\n", Seed);
  
  /* starts time measurement */
  start_timers();

  /* A solution is just a vector of int with the same size as the instance */
  currentSolution = (long int *)malloc(PSize * sizeof(long int));

  int isVND = (strcmp(pivotingRule, "--vnd") == 0);

  /* Assign pivoting function (not used for VND) */
  if (!isVND) {
    if (strcmp(pivotingRule, "--first") == 0) {
      pivotingFunction = firstImprovement;
    } else if (strcmp(pivotingRule, "--best") == 0) {
      pivotingFunction = bestImprovement;
    } else {
      printf("Invalid pivoting rule provided. Exiting.\n");
      exit(1);
    }
  }

  /* Assign initial solution function */
  if (strcmp(initialSolutionType, "--random") == 0) {
    initialSolutionFunction = createRandomSolution;
  }
  else if (strcmp(initialSolutionType, "--chenery") == 0) {
    initialSolutionFunction = cheneryWatanabe;
  }
  else {
    printf("Invalid initial solution type provided. Exiting.\n");
    exit(1);
  }

  /* Assign neighborhood move function (not used for VND) */
  if (!isVND) {
    if (strcmp(neighborhoodType, "--exchange") == 0) {
      neighborhoodMoveFunction = exchange;
    }
    else if (strcmp(neighborhoodType, "--insert") == 0) {
      neighborhoodMoveFunction = insert;
    }
    else if (strcmp(neighborhoodType, "--transpose") == 0) {
      neighborhoodMoveFunction = transpose;
    }
    else {
      printf("Invalid neighborhood type provided. Exiting.\n");
      exit(1);
    }
  }

  /* Generate initial solution */
  initialSolutionFunction(currentSolution);
  printf("Initial solution generated using %s method.\n", initialSolutionType);

  /* Compute cost of initial solution */
  cost = computeCost(currentSolution);
  initialCost = cost;
  printf("Cost of initial solution: %lld\n", cost);

  /* Optimize */
  iterations = 0;
  if (isVND) {
    /* VND: cycle through neighborhoods, resetting to first on any improvement */
    void (*vndMoves[3])(long int *, int, int);
    vndMoves[0] = transpose;
    if (strcmp(neighborhoodType, "--tei") == 0) {
      vndMoves[1] = exchange;
      vndMoves[2] = insert;
    } else { /* --tie */
      vndMoves[1] = insert;
      vndMoves[2] = exchange;
    }
    cost = vnd(currentSolution, vndMoves);
  } else {
    /* Standard iterative improvement */
    while (1) {
      candidateCost = pivotingFunction(currentSolution, neighborhoodMoveFunction);
      if (candidateCost <= cost)
        break;
      cost = candidateCost;
      iterations++;
    }
  }

  printf("Cost after optimization: %lld\n", cost);
  if (!isVND)
    printf("Optimization iterations: %d\n", iterations);

  /* Compare the two solutions */
  if (cost == initialCost)
    printf("Second solution is as good as first one\n");
  else if (cost > initialCost)
    printf("Second solution is better than first one\n");
  else
    printf("Second solution is worse than first one\n");

  printf("Time elapsed since we started the timer: %g\n\n", elapsed_time(VIRTUAL));

  /* Calculate relative percentage deviation related to bestKnown */
  double rpd = 0.0;
  if (bestKnown > 0) {
    rpd = 100.0 * (bestKnown - cost) / bestKnown;
    printf("Relative percentage deviation from best known solution: %.2f%%\n", rpd);
  } else {
    printf("Best known solution not available.\n");
  }

  /* Append one row to results.dat (write header if file is new) */
  {
    FILE *dat;
    int writeHeader = 0;
    dat = fopen("output/results.txt", "r");
    if (dat == NULL)
      writeHeader = 1;
    else
      fclose(dat);

    dat = fopen("output/results.txt", "a");
    if (dat != NULL) {
      if (writeHeader)
        fprintf(dat, "instance pivoting neighborhood initial cost rpd time\n");
      fprintf(dat, "%s %s %s %s %lld %.4f %g\n",
              instanceName,
              pivotingRule + 2,      /* skip leading "--" */
              neighborhoodType + 2,
              initialSolutionType + 2,
              cost, rpd, elapsed_time(VIRTUAL));
      fclose(dat);
    }
  }

  /* Append cost to a per-algorithm file (e.g. lop-first-transpose-random.dat).
     One cost value per line, used directly by the R analysis script. */
  {
    char algFileName[256];
    FILE *algFile;
    snprintf(algFileName, sizeof(algFileName), "output/lop-%s-%s-%s.txt",
             pivotingRule + 2,
             neighborhoodType + 2,
             initialSolutionType + 2);
    algFile = fopen(algFileName, "a");
    if (algFile != NULL) {
      fprintf(algFile, "%lld\n", cost);
      fclose(algFile);
    }
  }

  return 0;
}

