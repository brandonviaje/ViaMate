#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "Board.h"
#include "MyTypes.h"

// Globals
extern U64 pieceKeys[13][64]; // [PieceType][Square]
extern U64 sideKey;           // Random num to XOR when it's Black's turn
extern U64 castleKeys[16];    // Random num for castling rights 
extern U64 enPassantKeys[65];
extern U64 positionKey;

// current board's unique ID
extern U64 positionKey;

void InitZobrist();
U64 GeneratePosKey();       
void HashPiece(int piece, int square);
void HashCastle();
void HashSide();
void HashEnPassant();

#endif
