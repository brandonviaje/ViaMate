#include "../include/Transposition.h"

TranspositionTable TTable;

void InitTT(int mbSize)
{
    // raw byte calculation
    int bytes = mbSize * 1024 * 1024;
    int requestedEntries = bytes / sizeof(TTEntry);

    // align to nearest power of 2. makes indexing cheaper/safer
    TTable.numEntries = 1;
    while (TTable.numEntries <= requestedEntries)
    {
        TTable.numEntries <<= 1;
    }

    TTable.numEntries >>= 1; // overshot, step back

    if (TTable.entries != NULL)
        free(TTable.entries);

    TTable.entries = (TTEntry *)malloc(TTable.numEntries * sizeof(TTEntry));

    if (TTable.entries == NULL)
    {
        printf("Error: Failed to allocate TT!\n");
        exit(1);
    }

    ClearTT();
    // printf("TT initialized with %d entries (%zu MB)\n", TTable.numEntries, (TTable.numEntries * sizeof(TTEntry)) / (1024 * 1024));
}

void ClearTT()
{
    memset(TTable.entries, 0, TTable.numEntries * sizeof(TTEntry));
}

// read tt
int ReadTT(U64 positionKey, int *move, int *score, int alpha, int beta, int depth)
{
    // map key to index
    int index = positionKey & (TTable.numEntries - 1);

    // collision check
    if (TTable.entries[index].hashKey == positionKey)
    {
        *move = TTable.entries[index].move;

        // ignore shallow results
        if (TTable.entries[index].depth >= depth)
        {
            *score = TTable.entries[index].score;
            int flag = TTable.entries[index].flags;

            if (flag == TT_FLAG_EXACT)
            {
                return 1; // exact hit
            }
            if (flag == TT_FLAG_ALPHA && *score <= alpha)
            {
                return 1; // fail-low (upper bound)
            }
            if (flag == TT_FLAG_BETA && *score >= beta)
            {
                return 1; // fail-high (lower bound)
            }
        }
    }
    return 0; // miss
}

// store entry
void WriteTT(U64 positionKey, int move, int score, int depth, int flag)
{
    int index = positionKey & (TTable.numEntries - 1);

    if (TTable.entries[index].hashKey == positionKey && TTable.entries[index].depth > depth)
    {
        return; 
    }

    // always replace scheme
    TTable.entries[index].hashKey = positionKey;
    TTable.entries[index].score = score;
    TTable.entries[index].flags = flag;
    TTable.entries[index].depth = depth;
    TTable.entries[index].move = move;
}
