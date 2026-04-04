/*  Heuristic Optimization assignment, 2015.
        Adapted by Jérémie Dubois-Lacoste from the ILSLOP implementation
        of Tommaso Schiavinotto:
        ---
        ILSLOP Iterated Lcaol Search Algorithm for Linear Ordering Problem
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
#include <limits.h>

#include "optimization.h"
#include "instance.h"
#include "utilities.h"
#include <float.h>

#ifdef __MINGW32__
#define MAX_FLOAT FLT_MAX
#else
#define MAX_FLOAT MAXFLOAT
#endif

long int **CostMat;

// Check if the move function is transpose, which allows us to optimize the inner loop of the local search
static int isTransposeMove(void (*move)(long int *, int, int))
{
    return move == transpose;
}

/* Delta cost for swapping positions i and j (exchange or transpose).
   Only pairs involving position i or j change.
    Used also when move is transpose, since the delta is the same as for exchange of adjacent elements.
*/
long long int deltaExchange(long int *s, int i, int j)
{
    long int a = s[i], b = s[j];
    long long int delta = CostMat[b][a] - CostMat[a][b];
    int k;

    for (k = 0; k < i; k++)
        delta += CostMat[s[k]][b] - CostMat[s[k]][a];
    for (k = i + 1; k < j; k++)
        delta += (CostMat[b][s[k]] - CostMat[a][s[k]]) + (CostMat[s[k]][a] - CostMat[s[k]][b]);
    for (k = j + 1; k < PSize; k++)
        delta += CostMat[a][s[k]] - CostMat[b][s[k]];

    return delta;
}

/* Delta cost for moving element at position i to position j (insert).
   Only the relationships between the moved element and the elements it
   skips over change: they flip from CostMat[a][s[k]] to CostMat[s[k]][a]
   or vice versa. */
long long int deltaInsert(long int *s, int i, int j)
{
    long int a = s[i];
    long long int delta = 0;
    int k;

    if (i < j)
    {
        for (k = i + 1; k <= j; k++)
            delta += CostMat[s[k]][a] - CostMat[a][s[k]];
    }
    else
    {
        for (k = j; k < i; k++)
            delta += CostMat[a][s[k]] - CostMat[s[k]][a];
    }

    return delta;
}

/* Dispatch to the correct delta function based on the move type. */
long long int computeDelta(long int *s, void (*move)(long int *, int, int), int i, int j)
{
    if (move == insert)
        return deltaInsert(s, i, j);
    return deltaExchange(s, i, j); // covers both exchange and transpose
}

long long int computeCost(long int *s)
{
    int h, k;
    long long int sum;

    /* Diagonal value are not considered */
    for (sum = 0, h = 0; h < PSize; h++)
        for (k = h + 1; k < PSize; k++)
            sum += CostMat[s[h]][s[k]];
    return (sum);
}

// Generates a random solution by creating a random vector of size PSize and copying it to s
void createRandomSolution(long int *s)
{
    int j;
    long int *random;

    random = generate_random_vector(PSize);
    for (j = 0; j < PSize; j++)
    {
        s[j] = random[j];
    }
    free(random);
}

// Generates a solution using the Chenery-Watanabe heuristic
void cheneryWatanabe(long int *s)
{
    int i, j, k;
    long int *used;

    used = (long int *)calloc(PSize, sizeof(long int));
    if (used == NULL)
    {
        createRandomSolution(s);
        return;
    }

    for (k = 0; k < PSize; k++)
    {
        /* At step k, pick the unplaced element i with the highest attractiveness:
           sum of CostMat[i][j] for all remaining unplaced j != i */
        long long int bestScore = LLONG_MIN;
        int bestIndex = -1;

        for (i = 0; i < PSize; i++)
        {
            if (!used[i])
            {
                long long int score = 0;
                for (j = 0; j < PSize; j++)
                {
                    if (!used[j] && j != i)
                        score += CostMat[i][j];
                }
                if (score > bestScore)
                {
                    bestScore = score;
                    bestIndex = i;
                }
            }
        }

        s[k] = bestIndex;
        used[bestIndex] = 1;
    }

    free(used);
}

/*
    Performs a first improvement local search on the solution s using the specified move function.
    Returns the cost of the improved solution, or the original cost if no improvement was found.

    long int *s: the current solution to be improved
    void (*move)(long int *, int, int): a pointer to the move function to be applied (transpose, exchange, or insert)
*/
long long int firstImprovement(long int *s, void (*move)(long int *, int, int))
{
    int i, j;
    int transposeMove = isTransposeMove(move);
    long long int oldCost = computeCost(s);
    long long int delta;

    for (i = 0; i < PSize; i++)
    {
        int jStart = i + 1;
        int jEnd = transposeMove ? (i + 2) : PSize; // For transpose, only consider the adjacent element

        if (jStart >= PSize)
            continue;

        for (j = jStart; j < jEnd && j < PSize; j++)
        {
            delta = computeDelta(s, move, i, j);
            if (delta > 0)
            {
                move(s, i, j);
                return oldCost + delta;
            }
        }
    }
    return oldCost;
}

/*
    Performs a best improvement local search on the solution s using the specified move function.
    Returns the cost of the improved solution, or the original cost if no improvement was found.

    long int *s: the current solution to be improved
    void (*move)(long int *, int, int): a pointer to the move function to be applied (transpose, exchange, or insert)
*/
long long int bestImprovement(long int *s, void (*move)(long int *, int, int))
{
    int i, j;
    int transposeMove = isTransposeMove(move);
    int bestI = -1, bestJ = -1;
    long long int oldCost = computeCost(s);
    long long int delta, bestDelta = 0;

    for (i = 0; i < PSize; i++)
    {
        int jStart = i + 1;
        int jEnd = transposeMove ? (i + 2) : PSize;

        if (jStart >= PSize)
            continue;

        for (j = jStart; j < jEnd && j < PSize; j++)
        {
            delta = computeDelta(s, move, i, j);
            if (delta > bestDelta)
            {
                bestDelta = delta;
                bestI = i;
                bestJ = j;
            }
        }
    }

    if (bestI >= 0)
        move(s, bestI, bestJ);

    return oldCost + bestDelta;
}

/* Variable Neighbourhood Descent
    s: the current solution to be improved
    moves: an array of pointers to move functions representing the different neighborhoods to explore
    numMoves: the number of move functions in the moves array
    
    Returns the cost of the improved solution after exploring all neighborhoods.
*/
long long int vnd(long int *s, void (*moves[])(long int *, int, int))
{
    long long int cost = computeCost(s);
    long long int newCost;
    int i = 0;

    while (i < 3)
    {
        newCost = firstImprovement(s, moves[i]);
        if (newCost > cost)
        {
            cost = newCost;
            i = 0;  /* improvement found: restart from the first neighborhood */
        }
        else
        {
            i++;    /* no improvement: move to the next neighborhood */
        }
    }

    return cost;
}

/*
    Performs a transpose move on the solution s by swapping the element at index i with the element at index i+1.
    The placeholder parameter is not used in this move.

    long int *s: the current solution to be modified
    int i: the index of the first element to be swapped
    int placeholder: an unused parameter for compatibility with the move function signature
*/
void transpose(long int *s, int i, int placeholder)
{
    long int temp;
    int j = i + 1;

    if (j >= PSize)
        j -= PSize;

    temp = s[i];
    s[i] = s[j];
    s[j] = temp;
}

/*
    Performs an exchange move on the solution s by swapping the elements at indices i and j.

    long int *s: the current solution to be modified
    int i: the index of the first element to be swapped
    int j: the index of the second element to be swapped
*/
void exchange(long int *s, int i, int j)
{
    long int temp;

    temp = s[i];
    s[i] = s[j];
    s[j] = temp;
}

/*
    Performs an insert move on the solution s by moving the element at index i to index j, shifting the elements in between accordingly.

    long int *s: the current solution to be modified
    int i: the index of the element to be moved
    int j: the target index where the element should be moved
*/
void insert(long int *s, int i, int j)
{
    long int temp;
    int k;

    temp = s[i];
    if (i < j)
    {
        for (k = i; k < j; k++)
            s[k] = s[k + 1];
    }
    else
    {
        for (k = i; k > j; k--)
            s[k] = s[k - 1];
    }
    s[j] = temp;
}