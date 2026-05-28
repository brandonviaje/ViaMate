#include "../include/Zobrist.h"

U64 pieceKeys[13][64];
U64 sideKey;
U64 castleKeys[16];
U64 enPassantKeys[65]; 
U64 positionKey;

U64 XorShift64(U64 *state) 
{
    U64 x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

void InitZobrist() 
{
    U64 seed = 0x9E3779B97F4A7C15ULL;

    // fill piece keys
    for(int piece = 0; piece < 13; piece++) 
    {
        for(int sq = 0; sq < 64; sq++) 
        {
            pieceKeys[piece][sq] = XorShift64(&seed);
        }
    }

    // side key
    sideKey = XorShift64(&seed);

    // castle keys
    for(int i = 0; i < 16; i++) 
    {
        castleKeys[i] = XorShift64(&seed);
    }
    
    // en Passant keys  init all of them
    for(int i = 0; i < 120; i++) 
    {
        enPassantKeys[i] = XorShift64(&seed);
    }

    enPassantKeys[64] = 0ULL;
}

// Compute key from scratch 
U64 GeneratePosKey() 
{
    U64 finalKey = 0;

    // loop through all piece types
    for (int piece = 0; piece < 12; piece++) 
    {
        U64 bb = bitboards[piece];
        // scan the bitboard for pieces
        while (bb) 
        {
            int sq = __builtin_ctzll(bb); 
            finalKey ^= pieceKeys[piece][sq];
            bb &= bb - 1; 
        }
    }

    // hash Side
    if (side == WHITE) 
    {
        finalKey ^= sideKey;
    }

    // hash Castle
    finalKey ^= castleKeys[castle];

    // hash En Passant
    int epIndex = (enpassant == -1) ? 64 : enpassant;
    finalKey ^= enPassantKeys[epIndex];

    return finalKey;
}

// Zobrist Helpers

void HashPiece(int piece, int square) 
{
    positionKey ^= pieceKeys[piece][square];
}

void HashCastle() 
{
    positionKey ^= castleKeys[castle];
}

void HashSide() 
{
    positionKey ^= sideKey;
}

void HashEnPassant() 
{
    int index = (enpassant == -1) ? 64 : enpassant; 
    positionKey ^= enPassantKeys[index];
}
