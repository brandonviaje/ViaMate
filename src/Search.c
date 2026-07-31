#include "../include/Search.h"

S_SEARCHINFO info;
Move killerMoves[MAX_PLY][2];
Move pvTable[MAX_PLY][MAX_PLY];
int pvLength[MAX_PLY];

// get time in milliseconds from linux
int GetTimeMs()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

// checks if we ran out of time check every 2048 nodes to not slow things down
void CheckTime()
{
    if (info.nodes % 2048 == 0)
    {
        if (GetTimeMs() > info.stopTime)
        {
            info.stopped = 1;
        }
    }
}

void ClearSearch()
{
    info.nodes = 0;
    info.stopped = 0;
    info.startTime = GetTimeMs();
    memset(killerMoves, 0, sizeof(killerMoves));
}

// alpha beta search algorithm
int AlphaBeta(int alpha, int beta, int depth, int ply)
{
    // if we go too deep, just stop and evaluate
    if (ply >= MAX_PLY)
        return Evaluate();

    // check clock periodically
    if ((info.nodes & 2047) == 0)
        CheckTime();

    // abort if time is up
    if (info.stopped)
        return 0;

    // check the transposition table if we've seen this exact position before, use that score
    int score = -INF;
    int ttMovePacked = 0;
    int ttScore = -INF;

    if (ReadTT(positionKey, &ttMovePacked, &ttScore, alpha, beta, depth))
    {
        return ttScore;
    }

    // if we hit depth 0, run quiescence search
    if (depth == 0)
        return Quiescence(alpha, beta, ply);

    info.nodes++;

    MoveList list;
    GenerateLegalMoves(&list);

    int legalMoves = 0;
    int alphaOrig = alpha;
    Move bestMove = {0};

    // grab killer moves for sorting later
    Move k1 = killerMoves[ply][0];
    Move k2 = killerMoves[ply][1];

    // loop through all moves
    for (int i = 0; i < list.count; i++)
    {
        // pick best-looking move next
        PickNextMove(&list, i, k1, k2, ttMovePacked);

        Move m = list.moves[i];

        MakeMove(&list.moves[i]);

        // check for legality
        if (IsKingInCheck(side ^ 1))
        {
            UndoMove(&list.moves[i]);
            continue;
        }

        legalMoves++;

        // flip perspective and reduce depth
        score = -AlphaBeta(-beta, -alpha, depth - 1, ply + 1);

        UndoMove(&list.moves[i]);

        if (info.stopped)
            return 0;

        // if this move is too good, the opponent won't let us play it (beta cutoff)
        if (score >= beta)
        {
            // if it wasn't a capture, remember it as a "killer" move
            if (m.captured == -1)
            {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = m;
            }

            // save to transp table (lower bound)
            int packed = m.from | (m.to << 6);
            WriteTT(positionKey, packed, beta, depth, TT_FLAG_BETA);

            return beta;
        }

        // if we found a new best move, update alpha
        if (score > alpha)
        {
            alpha = score;
            bestMove = m;
        }
    }

    // checkmate or stalemate
    if (legalMoves == 0)
    {
        if (IsKingInCheck(side))
            return -MATE + ply; // checkmate
        else
            return 0; // stalemate
    }

    // save result to transp table
    int packedBest = bestMove.from | (bestMove.to << 6);

    if (alpha > alphaOrig)
    {
        WriteTT(positionKey, packedBest, alpha, depth, TT_FLAG_EXACT);
    }
    else
    {
        WriteTT(positionKey, packedBest, alpha, depth, TT_FLAG_ALPHA);
    }

    return alpha;
}

// handle search process, start at depth 1 and goes deeper until time runs out
void SearchPosition(int maxDepth, int timeAllocatedMs)
{
    ClearSearch();
    info.stopTime = info.startTime + timeAllocatedMs;

    Move bestMove = {0};
    int bestScore = 0;

    // loop from depth 1 up to maxDepth
    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++)
    {
        if (GetTimeMs() >= info.stopTime)
            break;

        int alpha = -INF;
        int beta = INF;

        MoveList list;
        GenerateLegalMoves(&list);

        Move depthBestMove = {0};
        int depthBestScore = -INF;

        // check if we found a best move in the previous depth
        int prevBestPacked = 0;
        if (currentDepth > 1)
        {
            prevBestPacked = bestMove.from | (bestMove.to << 6);
        }

        for (int i = 0; i < list.count; i++)
        {
            // use the previous best move to search first
            PickNextMove(&list, i, (Move){0}, (Move){0}, prevBestPacked);

            MakeMove(&list.moves[i]);

            if (IsKingInCheck(side ^ 1))
            {
                UndoMove(&list.moves[i]);
                continue;
            }

            // start the recursive search
            int score = -AlphaBeta(-beta, -alpha, currentDepth - 1, 1);

            UndoMove(&list.moves[i]);

            if (info.stopped)
                break;

            if (score > depthBestScore)
            {
                depthBestScore = score;
                depthBestMove = list.moves[i];
                alpha = score;
            }
        }

        if (info.stopped)
            break;

        bestMove = depthBestMove;
        bestScore = depthBestScore;

        // print info for the gui
        long timeSpent = GetTimeMs() - info.startTime;
        if (timeSpent == 0)
            timeSpent = 1;

        printf("info depth %d score cp %d nodes %ld time %ld nps %ld pv",
               currentDepth, bestScore, info.nodes, timeSpent, (info.nodes * 1000) / timeSpent);

        // print the best move found so far
        char f1 = 'a' + (bestMove.from % 8);
        char r1 = '1' + (bestMove.from / 8);
        char f2 = 'a' + (bestMove.to % 8);
        char r2 = '1' + (bestMove.to / 8);
        printf(" %c%c%c%c", f1, r1, f2, r2);

        if (bestMove.flags & FLAG_PROMOTION)
            printf("%c", GetPromotionChar(bestMove.promotion));

        printf("\n");
    }

    // tell the gui our final decision
    char f1 = 'a' + (bestMove.from % 8);
    char r1 = '1' + (bestMove.from / 8);
    char f2 = 'a' + (bestMove.to % 8);
    char r2 = '1' + (bestMove.to / 8);
    printf("bestmove %c%c%c%c", f1, r1, f2, r2);

    if (bestMove.flags & FLAG_PROMOTION)
        printf("%c", GetPromotionChar(bestMove.promotion));

    printf("\n");
}

// quiescence search (resolve noisy moves)
int Quiescence(int alpha, int beta, int ply)
{
    if (ply >= MAX_PLY)
        return Evaluate();

    if ((info.nodes & 2047) == 0)
        CheckTime();

    if (info.stopped)
        return 0;

    info.nodes++;

    // stand-pat: assume we don't have to capture back if we are already winning
    int score = Evaluate();

    if (score >= beta)
        return beta;
    if (score > alpha)
        alpha = score;

    MoveList list;
    GenerateLegalMoves(&list);

    for (int i = 0; i < list.count; i++)
    {
        // no killer moves needed here
        PickNextMove(&list, i, (Move){0}, (Move){0}, 0);
        Move m = list.moves[i];

        // ignore quiet moves, only check captures and promotions
        if (m.captured == -1 && !(m.flags & FLAG_PROMOTION))
            continue;

        MakeMove(&list.moves[i]);

        if (IsKingInCheck(side ^ 1))
        {
            UndoMove(&list.moves[i]);
            continue;
        }

        int tempScore = -Quiescence(-beta, -alpha, ply + 1);

        UndoMove(&list.moves[i]);

        if (info.stopped)
            return 0;

        if (tempScore >= beta)
            return beta;
        if (tempScore > alpha)
            alpha = tempScore;
    }
    return alpha;
}



void RunBenchmark()
{
    printf("--- Starting Search Optimization Benchmark ---\n");
    
    // kiwipete position
    char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    
    ParseFEN(fen); 

    int testDepth = 7;
    printf("Searching Kiwipete to Depth %d...\n", testDepth);
    
    // run search with a massive time limit so it goes to depth 7
    SearchPosition(testDepth, 9999999); 

    // calculate effective branching factor (Nodes ^ (1 / Depth))
    double ebf = pow((double)info.nodes, 1.0 / testDepth);

    printf("\n--- Benchmark Results ---\n");
    printf("Total Nodes Searched: %ld\n", info.nodes);
    printf("Effective Branching Factor: %.2f\n", ebf);
    printf("-------------------------\n");
}
