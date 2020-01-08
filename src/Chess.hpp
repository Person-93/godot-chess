#pragma once

#include <array>
#include <utility>
#include <unordered_set>
#include <sqlite_modern_cpp.h>

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

    Chess( const Chess& other );

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

    using LegalMoves = std::vector<Move>;

    [[nodiscard]] LegalMoves legalMoves() const;

private:
    Cell& atLocation( std::pair<int, int> location );

    struct MovesDatabase {
        MovesDatabase();

        MovesDatabase( const MovesDatabase& ) = delete;

        MovesDatabase( MovesDatabase&& ) = default;

        struct Inserter {
            sqlite::database_binder binder;

            void insert( const Move& move );
        };

        Inserter inserter();

        sqlite::database db{ ":memory:" };
    };

    void calculateLegalMoves( MovesDatabase::Inserter& inserter );

    void calculatePawnMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite );

    /**
     * Helper function that checks all the squares in one direction from the given location and inserts them into
     * legalMoves_ until it finds an occupied square. The occupied square will be added if it is occupied by a piece
     * of the opposite color
     * @param location
     * @param isWhite
     * @param xIncrement
     * @param yIncrement
     */
    void checkInDirection( MovesDatabase::Inserter& inserter,
                           std::pair<int, int> location,
                           bool isWhite,
                           void (* xIncrement)( int& ),
                           void (* yIncrement)( int& ));

    void calculateRookMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite );

    void calculateKnightMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite );

    void calculateBishopMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite );

    void calculateQueenMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite );

    void calculateKingMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite );

    BoardState            boardState_;
    mutable MovesDatabase moves;
    bool                  whiteTurn   = true;
    bool                  inCheck     = false;
    std::pair<int, int>   whiteKingLocation;
    std::pair<int, int>   blackKingLocation;
};
