#include "../include/Perft.h"

U64 Perft(int depth)
{
    // bulk count at depth 1, just need to know how many legal moves exist from this position
    if (depth == 1) 
    {
        MoveList list;
        GenerateLegalMoves(&list);
        return list.count;
    }

    // base case
    if (depth <= 0)
        return 1;

    MoveList list;
    GenerateLegalMoves(&list);

    U64 nodes = 0;

    for (int i = 0; i < list.count; i++)
    {
        MakeMove(&list.moves[i]);
        nodes += Perft(depth - 1);
        UndoMove(&list.moves[i]);
    }

    return nodes;
}

void PerftDivide(int depth)
{
    MoveList list;
    GenerateLegalMoves(&list);

    U64 total = 0;

    for (int i = 0; i < list.count; i++)
    {
        MakeMove(&list.moves[i]);

        U64 nodes = Perft(depth - 1);

        char fromFile = 'a' + (list.moves[i].from % 8);
        char fromRank = '1' + (list.moves[i].from / 8);
        char toFile = 'a' + (list.moves[i].to % 8);
        char toRank = '1' + (list.moves[i].to / 8);

        printf("%c%c%c%c: %llu\n", fromFile, fromRank, toFile, toRank, nodes);
        total += nodes;

        UndoMove(&list.moves[i]);
    }

    printf("Total nodes: %llu\n", total);
}
