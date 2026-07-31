#include "../include/uci.h"

extern int side;

// handle go command
void ParseGo(char *line)
{
    int depth = -1;
    int moveTime = -1;
    int time = -1;
    int inc = 0;
    char *ptr = NULL;

    // check if we should search forever until told to stop
    if ((ptr = strstr(line, "infinite")))
    {
        time = 99999999;
    }

    // check if we have a specific depth target
    if ((ptr = strstr(line, "depth")))
    {
        depth = atoi(ptr + 6);
    }

    // check if we have a fixed time per move
    if ((ptr = strstr(line, "movetime")))
    {
        moveTime = atoi(ptr + 9);
    }

    // see how much time white has left
    if ((ptr = strstr(line, "wtime")) && side == WHITE)
    {
        time = atoi(ptr + 6);
    }

    // see how much time black has left
    if ((ptr = strstr(line, "btime")) && side == BLACK)
    {
        time = atoi(ptr + 6);
    }

    // check for time increment (bonus time per move)
    if ((ptr = strstr(line, "winc")) && side == WHITE)
    {
        inc = atoi(ptr + 5);
    }
    if ((ptr = strstr(line, "binc")) && side == BLACK)
    {
        inc = atoi(ptr + 5);
    }

    // decide how long to spend on this move
    int timeToSpend = 0;

    if (moveTime != -1)
    {
        timeToSpend = moveTime;
        if (timeToSpend > 10)
            timeToSpend -= 10; // safety buffer so we don't flag fall
    }
    else if (time != -1)
    {
        // divide remaining time by 20 moves plus the increment
        timeToSpend = (time / 20) + (inc / 2);

        // cap it so we don't think for too long in the opening
        if (timeToSpend > 10000)
            timeToSpend = 10000;
    }
    else
    {
        timeToSpend = 1000; // default to 1 second if we have no clue
    }

    // if depth isn't set, just go deep enough
    if (depth == -1)
        depth = 64;

    // printf("Thinking for %d ms...\n", timeToSpend);
    SearchPosition(depth, timeToSpend);
}

// handle position command
void ParsePosition(char *lineIn)
{
    // skip the word "position "
    lineIn += 9;
    char *ptrChar = lineIn;

    // handle standard starting position
    if (strncmp(lineIn, "startpos", 8) == 0)
    {
        ParseFEN(starting_position);
    }
    else
    {
        // handle custom position string
        ptrChar = strstr(lineIn, "fen");
        if (ptrChar == NULL)
        {
            ParseFEN(starting_position);
        }
        else
        {
            ptrChar += 4; // skip "fen "
            ParseFEN(ptrChar);
        }
    }

    // apply any moves that have happened since the start
    ptrChar = strstr(lineIn, "moves");

    if (ptrChar != NULL)
    {
        ptrChar += 6; // skip "moves "

        while (*ptrChar)
        {
            // find the move in our list of legal moves
            int moveIndex = ParseMove(ptrChar, side);

            if (moveIndex == -1)
                break;

            // make the move on our board
            MoveList list[1];
            GenerateLegalMoves(list);
            MakeMove(&list->moves[moveIndex]);

            // jump to next move in the string
            while (*ptrChar && *ptrChar != ' ')
                ptrChar++;
            ptrChar++; // skip the space
        }
    }

    // refresh bitboards just in case
    UpdateBitboards();
}

// convert string into an engine move
int ParseMove(char *char_ptr, int side)
{
    int from = (char_ptr[0] - 'a') + ((char_ptr[1] - '1') * 8);
    int to = (char_ptr[2] - 'a') + ((char_ptr[3] - '1') * 8);

    MoveList list[1];

    GenerateLegalMoves(list);

    for (int i = 0; i < list->count; i++)
    {
        Move m = list->moves[i];
        if (m.from == from && m.to == to)
        {
            // check if we are promoting a pawn
            int promoted = -1;
            if (char_ptr[4])
            {
                if (char_ptr[4] == 'q')
                    promoted = (side == WHITE) ? Q : q;
                else if (char_ptr[4] == 'r')
                    promoted = (side == WHITE) ? R : r;
                else if (char_ptr[4] == 'b')
                    promoted = (side == WHITE) ? B : b;
                else if (char_ptr[4] == 'n')
                    promoted = (side == WHITE) ? N : n;

                // make sure it matches the promotion type we want
                if (m.promotion != promoted)
                    continue;
            }

            return i;
        }
    }
    return -1;
}

void UciLoop()
{
    // disable buffering so output is instant
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char line[INPUT_BUFFER];

    while (1)
    {
        memset(line, 0, sizeof(line));

        // wait for input
        if (!fgets(line, INPUT_BUFFER, stdin))
            continue;

        if (line[0] == '\n')
            continue;

        // gui check if we are awake
        if (strncmp(line, "isready", 7) == 0)
        {
            printf("readyok\n");
            continue;
        }
        else if (strncmp(line, "position", 8) == 0)
        {
            ParsePosition(line);
        }
        else if (strncmp(line, "ucinewgame", 10) == 0)
        {
            ParsePosition("position startpos\n");
        }
        else if (strncmp(line, "go", 2) == 0)
        {
            ParseGo(line);
        }
        else if (strncmp(line, "quit", 4) == 0)
        {
            break;
        }
        else if (strncmp(line, "bench", 5) == 0)
        {
            RunBenchmark();
        }
        else if (strncmp(line, "uci", 3) == 0)
        {
            printf("id name Viaje 1.0\n");
            printf("id author Brandon Viaje\n");
            printf("option name Move Overhead type spin default 10 min 0 max 5000\n");
            printf("option name Threads type spin default 1 min 1 max 512\n");
            printf("option name Hash type spin default 16 min 1 max 32768\n");
            printf("option name SyzygyPath type string default <empty>\n");
            printf("option name UCI_ShowWDL type check default false\n");
            printf("uciok\n");
        }
        else if (strncmp(line, "printboard", 3) == 0)
        {
            PrintBitBoard(occupied);
        }
    }
}
