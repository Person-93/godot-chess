#pragma once

#include <array>
#include <utility>
#include <unordered_set>

class Chess {
public:
    enum class Pieces {
        PAWN = 0, ROOK = 1, KNIGHT = 2, BISHOP = 3, QUEEN = 4, KING = 5
    };

    enum class State {
        EMPTY = 0, WHITE = 1, BLACK = 2
    };

    struct Cell {
        State  state;
        Pieces piece;
    };

    using BoardState = std::array<std::array<Cell, 8>, 8>;

    Chess();

    [[nodiscard]] const BoardState& boardState() const { return boardState_; }

    bool move( std::pair<int, int> start, std::pair<int, int> end );

    [[nodiscard]] bool isWhiteTurn() const { return whiteTurn; }

    [[nodiscard]] bool isInCheck() const { return inCheck; }

    struct Move {
        std::pair<int, int> start;
        std::pair<int, int> end;
    };

    friend bool operator==( const Move& a, const Move& b );

    friend bool operator!=( const Move& a, const Move& b ) { return !( a == b ); }

    struct HashMove {
        size_t operator()( const Move& move ) const;
    };

    using LegalMoves = std::unordered_set<Move, HashMove>;

    [[nodiscard]] const LegalMoves& legalMoves() const { return legalMoves_; }

private:
    Cell& atLocation( std::pair<int, int> location );

    void calculateLegalMoves();

    void calculatePawnMoves( std::pair<int, int> location, bool isWhite );

    /**
     * Helper function that checks all the squares in one direction from the given location and inserts them into
     * legalMoves_ until it finds an occupied square. The occupied square will be added if it is occupied by a piece
     * of the opposite color
     * @param location
     * @param isWhite
     * @param xIncrement
     * @param yIncrement
     */
    void checkInDirection( std::pair<int, int> location,
                           bool isWhite,
                           void (* xIncrement)( int& ),
                           void (* yIncrement)( int& ));

    void calculateRookMoves( std::pair<int, int> location, bool isWhite );

    void calculateKnightMoves( std::pair<int, int> location, bool isWhite );

    void calculateBishopMoves( std::pair<int, int> location, bool isWhite );

    void calculateQueenMoves( std::pair<int, int> location, bool isWhite );

    void calculateKingMoves( std::pair<int, int> location, bool isWhite );

    BoardState boardState_;
    LegalMoves legalMoves_;
    bool       whiteTurn = true;
    bool       inCheck   = false;
};
