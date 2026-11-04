#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────
//  Data types
// ─────────────────────────────────────────────────────────────
struct HanoiMove {
    int  disk;
    char from;
    char to;
};

// ─────────────────────────────────────────────────────────────
//  Core recursive solver
//  Fills `moves` with the sequence of moves needed to transfer
//  n disks from `from` to `to` using `aux`.
// ─────────────────────────────────────────────────────────────
inline void solveTowerOfHanoi(int n, char from, char to, char aux,
                               std::vector<HanoiMove>& moves) {
    if (n <= 0) return;
    if (n == 1) {
        moves.push_back({1, from, to});
        return;
    }
    solveTowerOfHanoi(n - 1, from, aux, to,  moves); // move n-1 disks to aux
    moves.push_back({n, from, to});                   // move largest disk to target
    solveTowerOfHanoi(n - 1, aux,  to, from, moves); // move n-1 disks from aux to target
}

// ─────────────────────────────────────────────────────────────
//  Returns 2^n - 1 (total moves / recursive calls)
// ─────────────────────────────────────────────────────────────
inline int getRecursiveCalls(int n) {
    if (n < 0) throw std::invalid_argument("n must be non-negative");
    return (1 << n) - 1; // 2^n - 1
}

// ─────────────────────────────────────────────────────────────
//  Returns a JSON string describing all moves for n disks
// ─────────────────────────────────────────────────────────────
inline std::string getHanoiJSON(int n) {
    if (n < 0) throw std::invalid_argument("n must be non-negative");

    std::vector<HanoiMove> moves;
    moves.reserve(getRecursiveCalls(n));
    solveTowerOfHanoi(n, 'A', 'C', 'B', moves);

    std::ostringstream oss;
    oss << "{"
        << "\"disks\":" << n << ","
        << "\"totalMoves\":" << moves.size() << ","
        << "\"recursiveCalls\":" << getRecursiveCalls(n) << ","
        << "\"pegFrom\":\"A\","
        << "\"pegTo\":\"C\","
        << "\"pegAux\":\"B\","
        << "\"moves\":[";

    for (size_t i = 0; i < moves.size(); i++) {
        if (i > 0) oss << ",";
        oss << "{"
            << "\"step\":" << (i + 1) << ","
            << "\"disk\":" << moves[i].disk << ","
            << "\"from\":\"" << moves[i].from << "\","
            << "\"to\":\"" << moves[i].to << "\""
            << "}";
    }

    oss << "]}";
    return oss.str();
}
