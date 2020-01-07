#include "Chess.hpp"


Chess::Chess() : boardState_{} {
    boardState_[ 0 ][ 0 ] = { State::BLACK, Pieces::ROOK };
    boardState_[ 0 ][ 1 ] = { State::BLACK, Pieces::KNIGHT };
    boardState_[ 0 ][ 2 ] = { State::BLACK, Pieces::BISHOP };
    boardState_[ 0 ][ 3 ] = { State::BLACK, Pieces::QUEEN };
    boardState_[ 0 ][ 4 ] = { State::BLACK, Pieces::KING };
    boardState_[ 0 ][ 5 ] = { State::BLACK, Pieces::BISHOP };
    boardState_[ 0 ][ 6 ] = { State::BLACK, Pieces::KNIGHT };
    boardState_[ 0 ][ 7 ] = { State::BLACK, Pieces::ROOK };

    for ( int i = 0; i < 8; ++i ) {
        boardState_[ 1 ][ i ] = { State::BLACK, Pieces::PAWN };
        boardState_[ 6 ][ i ] = { State::WHITE, Pieces::PAWN };
    }

    boardState_[ 7 ] = boardState_[ 0 ];
    for ( int i = 0; i < 8; ++i ) {
        boardState_[ 7 ][ i ].state = State::WHITE;
    }

    calculateLegalMoves();
}

bool Chess::move( std::pair<int, int> start, std::pair<int, int> end ) {
    if ( start.first < 0 || start.first > 7 ) return false;
    if ( start.second < 0 || start.second > 7 ) return false;
    if ( end.first < 0 || end.first > 7 ) return false;
    if ( end.second < 0 || end.second > 7 ) return false;
    if ( legalMoves_.find( { start, end } ) == legalMoves_.end()) return false;

    auto& startCell = atLocation( start );
    auto& endCell   = atLocation( end );
    endCell = startCell;
    startCell.state = State::EMPTY;
    whiteTurn = !whiteTurn;
    calculateLegalMoves();
    return true;
}

void Chess::calculateLegalMoves() {
    legalMoves_.clear();
    for ( int i = 0; i < 8; ++i ) {
        for ( int j = 0; j < 8; ++j ) {
            auto& currentCell = boardState_[ i ][ j ];
            switch ( currentCell.state ) {
                case State::EMPTY:continue;
                case State::WHITE:if ( !whiteTurn ) continue;
                    break;
                case State::BLACK:if ( whiteTurn ) continue;
            }

            switch ( currentCell.piece ) {
                case Pieces::PAWN:calculatePawnMoves( { i, j }, whiteTurn );
                    break;
                case Pieces::ROOK:calculateRookMoves( { i, j }, whiteTurn );
                    break;
                case Pieces::KNIGHT:calculateKnightMoves( { i, j }, whiteTurn );
                    break;
                case Pieces::BISHOP:calculateBishopMoves( { i, j }, whiteTurn );
                    break;
                case Pieces::QUEEN:calculateQueenMoves( { i, j }, whiteTurn );
                    break;
                case Pieces::KING:calculateKingMoves( { i, j }, whiteTurn );
                    break;
            }
        }
    }
}

void Chess::calculatePawnMoves( std::pair<int, int> location, bool isWhite ) {
    if ( isWhite ) {
        std::pair<int, int> oneForward = { location.first - 1, location.second };
        if ( atLocation( oneForward ).state == State::EMPTY )
            legalMoves_.insert( { location, oneForward } );

        // can move two spaces if in starting position
        if ( location.first == 6 ) {
            std::pair<int, int> twoForward = { location.first - 2, location.second };
            if ( atLocation( twoForward ).state == State::EMPTY )
                legalMoves_.insert( { location, twoForward } );
        }

        // check if it can capture
        {
            std::pair<int, int> diagonal = { location.first - 1, location.second - 1 };
            if ( atLocation( diagonal ).state == State::BLACK )
                legalMoves_.insert( { location, diagonal } );
        }
        // check other diagonal
        {
            std::pair<int, int> diagonal = { location.first - 1, location.second + 1 };
            if ( atLocation( diagonal ).state == State::BLACK )
                legalMoves_.insert( { location, diagonal } );
        }
    }
    else {
        std::pair<int, int> oneForward = { location.first + 1, location.second };
        if ( atLocation( oneForward ).state == State::EMPTY )
            legalMoves_.insert( { location, oneForward } );

        // can move two spaces if in starting position
        if ( location.first == 1 ) {
            std::pair<int, int> twoForward = { location.first + 2, location.second };
            if ( atLocation( twoForward ).state == State::EMPTY )
                legalMoves_.insert( { location, twoForward } );
        }

        // check if it can capture
        {
            std::pair<int, int> diagonal = { location.first + 1, location.second - 1 };
            if ( atLocation( diagonal ).state == State::WHITE )
                legalMoves_.insert( { location, diagonal } );
        }
        // check other diagonal
        {
            std::pair<int, int> diagonal = { location.first + 1, location.second + 1 };
            if ( atLocation( diagonal ).state == State::WHITE )
                legalMoves_.insert( { location, diagonal } );
        }
    }
}

namespace {
    bool validateLocation( std::pair<int, int> location ) {
        return location.first >= 0 && location.first < 8 &&
               location.second >= 0 && location.second < 8;
    };
}

void Chess::checkInDirection( std::pair<int, int> location,
                              bool isWhite,
                              void (* xIncrement)( int& ),
                              void (* yIncrement)( int& )) {
    auto start = location;
    xIncrement( location.first );
    yIncrement( location.second );
    for ( bool isEmpty = true;
          isEmpty && validateLocation( location );
          xIncrement( location.first ), yIncrement( location.second )) {
        switch ( atLocation( location ).state ) {
            case State::EMPTY:legalMoves_.insert( { start, location } );
                break;
            case State::WHITE:
                if ( !isWhite )
                    legalMoves_.insert( { start, location } );
                isEmpty = false;
                break;
            case State::BLACK:
                if ( isWhite )
                    legalMoves_.insert( { start, location } );
                isEmpty = false;
                break;
        }
    }
}

void Chess::calculateRookMoves( std::pair<int, int> location, bool isWhite ) {
    checkInDirection( location, isWhite, []( int& i ) { ++i; }, []( int& ) {} );
    checkInDirection( location, isWhite, []( int& i ) { --i; }, []( int& ) {} );
    checkInDirection( location, isWhite, []( int& ) {}, []( int& i ) { ++i; } );
    checkInDirection( location, isWhite, []( int& ) {}, []( int& i ) { --i; } );
}

void Chess::calculateKnightMoves( std::pair<int, int> location, bool isWhite ) {
    auto sameColor = isWhite ? State::WHITE : State::BLACK;
    {
        std::pair<int, int> destination = { location.first + 2, location.second + 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first + 2, location.second - 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 2, location.second + 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 2, location.second - 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first + 1, location.second + 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first + 1, location.second - 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 1, location.second + 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 1, location.second - 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    }
}

void Chess::calculateBishopMoves( std::pair<int, int> location, bool isWhite ) {
    checkInDirection( location, isWhite, []( int& i ) { ++i; }, []( int& i ) { ++i; } );
    checkInDirection( location, isWhite, []( int& i ) { --i; }, []( int& i ) { ++i; } );
    checkInDirection( location, isWhite, []( int& i ) { ++i; }, []( int& i ) { --i; } );
    checkInDirection( location, isWhite, []( int& i ) { --i; }, []( int& i ) { --i; } );
}

void Chess::calculateQueenMoves( std::pair<int, int> location, bool isWhite ) {
    calculateBishopMoves( location, isWhite );
    calculateRookMoves( location, isWhite );
}

void Chess::calculateKingMoves( std::pair<int, int> location, bool isWhite ) {
    const auto checkDirection = [ & ]( void(* xIncrement)( int& ), void(* yIncrement)( int& )) {
        auto destination = location;
        xIncrement( destination.first );
        yIncrement( destination.second );
        auto sameColor = isWhite ? State::WHITE : State::BLACK;
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            legalMoves_.insert( { location, destination } );
    };
    checkDirection( []( int& i ) { ++i; }, []( int& ) {} );
    checkDirection( []( int& i ) { --i; }, []( int& ) {} );
    checkDirection( []( int& ) {}, []( int& i ) { ++i; } );
    checkDirection( []( int& ) {}, []( int& i ) { --i; } );
    checkDirection( []( int& i ) { ++i; }, []( int& i ) { ++i; } );
    checkDirection( []( int& i ) { ++i; }, []( int& i ) { --i; } );
    checkDirection( []( int& i ) { --i; }, []( int& i ) { ++i; } );
    checkDirection( []( int& i ) { --i; }, []( int& i ) { --i; } );
}

bool operator==( const Chess::Move& a, const Chess::Move& b ) {
    return a.start == b.start && a.end == b.end;
}

Chess::Cell& Chess::atLocation( std::pair<int, int> location ) {
    return boardState_[ location.first ][ location.second ];
}

namespace {
    template< class T >
    void hash_combine( std::size_t& seed, const T& v ) {
        std::hash<T> hasher{};
        seed ^= hasher( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 ); // NOLINT(hicpp-signed-bitwise)
    }
}

size_t Chess::HashMove::operator()( const Chess::Move& move ) const {
    size_t seed = std::hash<int>{}( move.start.first );
    hash_combine( seed, move.start.second );
    hash_combine( seed, move.end.first );
    hash_combine( seed, move.end.second );
    return seed;
}
