# ♟️ Chess Engine

A high-performance chess engine written in **C**, built completely from scratch with a focus on **efficient board representation**, **legal move generation**, and **modern search techniques**.  
The engine is deployed as an autonomous bot on **Lichess**, playing real-time games via their API.

---

## About the Project

This project explores the core systems behind competitive chess engines, emphasizing correctness, speed, and extensibility.  
It serves both as a learning exercise in low-level systems programming and as a foundation for future engine strength improvements.

### Key Features
- **Bitboard-based board representation** for fast move computation
- **Fully legal move generation**, including:
  - Castling
  - En passant
  - Promotions
  - Check and pin validation
- **Alpha–beta search with iterative deepening**
- **Search optimizations** including move ordering, quiescence search, and transposition tables
- **Deployed as a Lichess bot**, handling asynchronous I/O, time controls, and live game states

---

## Lichess Bot

The engine is live on **Lichess** as an automated bot:
- Plays rated and unrated games
- Manages real-time clocks and API events
- Communicates asynchronously using the Lichess Bot API

> This deployment validates engine correctness under real competitive conditions.

---

## Built With
- **C** — low-level performance and memory control
- **Make** — build automation
- **Lichess API** — bot deployment and game management

---

## Getting Started

Follow the steps below to build and run the engine locally.

### Prerequisites
- GCC or Clang
- Make

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/brandonviaje/ChessEngine.git
cd ChessEngine/src
```

2. **Build the Engine**
```bash
make 
```

3. **Run the Engine**
```bash
./viaje_Engine
```

**VS my bot [here](https://lichess.org/@/ViajeBot):** 

*Once you click the link, press “Challenge” to start!*



