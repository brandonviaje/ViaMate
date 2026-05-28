#ifndef TT_H
#define TT_H

#include "Main.h"

#define TT_FLAG_EXACT 0 // exact score
#define TT_FLAG_ALPHA 1 // Upper Bound
#define TT_FLAG_BETA 2  // Lower Bound

typedef struct
{
    U64 hashKey; //  zobrist Key
    int16_t score;   //  evaluation score
    uint16_t move;    //  best move found
    int8_t depth;
    uint8_t flags;   // EXACT, ALPHA, or BETA
} TTEntry;

typedef struct
{
    TTEntry *entries;
    int numEntries;
} TranspositionTable;

extern TranspositionTable TTable;

// Functions
void InitTT(int mbSize);
void ClearTT();
int ReadTT(U64 positionKey, int *move, int *score, int alpha, int beta, int depth);
void WriteTT(U64 positionKey, int move, int score, int depth, int flag);

#endif
