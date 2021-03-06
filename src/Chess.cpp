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

    whiteKingLocation = { 7, 4 };
    blackKingLocation = { 0, 4 };

    boardState_[ 7 ] = boardState_[ 0 ];
    for ( int i        = 0; i < 8; ++i ) {
        boardState_[ 7 ][ i ].state = State::WHITE;
    }
    auto      inserter = moves.inserter();
    calculateLegalMoves( inserter, whiteTurn );
}

Chess::Chess( const Chess& other ) :
        boardState_{ other.boardState_ },
        whiteTurn{ other.whiteTurn },
        inCheck{ other.inCheck },
        inCheckmate{ other.inCheckmate },
        whiteKingLocation{ other.whiteKingLocation },
        blackKingLocation{ other.blackKingLocation } {
    auto inserter = moves.inserter();
    calculateLegalMoves( inserter, whiteTurn );
}

bool Chess::move( std::pair<int, int> start, std::pair<int, int> end, bool extendedChecks ) {
    if ( inCheckmate ) return false;
    if ( inStalemate ) return false;

    // make sure move stars and ends on the board
    if ( start.first < 0 || start.first > 7 ) return false;
    if ( start.second < 0 || start.second > 7 ) return false;
    if ( end.first < 0 || end.first > 7 ) return false;
    if ( end.second < 0 || end.second > 7 ) return false;

    // check if move is in database
    {
        bool moveFound;
        moves.db << "SELECT EXISTS(SELECT * FROM moves WHERE start_x = ? AND start_y = ? AND end_x = ? AND end_y = ?);"
                 << start.first << start.second << end.first << end.second
                 >> moveFound;
        if ( !moveFound ) return false;
    }

    // make the move
    auto& startCell = atLocation( start );
    auto& endCell   = atLocation( end );
    auto oldEndCell = endCell;
    endCell = startCell;
    startCell.state = State::EMPTY;

    // if a king moved, update location
    if ( endCell.piece == Pieces::KING ) {
        if ( whiteTurn )
            whiteKingLocation = end;
        else
            blackKingLocation = end;
    }

    // make sure the move doesn't leave the player in check
    {
        MovesDatabase nextTurnMoves{};
        auto          inserter = nextTurnMoves.inserter();
        calculateLegalMoves( inserter, !whiteTurn );
        bool putInCheck;
        auto kingLocation      = whiteTurn ? whiteKingLocation : blackKingLocation;
        nextTurnMoves.db << "SELECT EXISTS(SELECT * FROM moves WHERE end_x = ? AND end_y = ?);"
                         << kingLocation.first << kingLocation.second
                         >> putInCheck;
        if ( putInCheck ) {
            // undo the move
            startCell = endCell;
            endCell   = oldEndCell;
            if ( startCell.piece == Pieces::KING ) {
                if ( whiteTurn )
                    whiteKingLocation = start;
                else
                    blackKingLocation = start;
            }
            return false;
        }
    }

    // if castling move the rook appropriately
    if ( endCell.piece == Pieces::KING ) {
        if ( start == std::pair{ 0, 4 } && end == std::pair{ 0, 2 } ) {
            atLocation( { 0, 3 } ) = { State::BLACK, Pieces::ROOK };
            atLocation( { 0, 0 } ).state = State::EMPTY;
        }
        else if ( start == std::pair{ 0, 4 } && end == std::pair{ 0, 6 } ) {
            atLocation( { 0, 5 } ) = { State::BLACK, Pieces::ROOK };
            atLocation( { 0, 7 } ).state = State::EMPTY;
        }
        else if ( start == std::pair{ 7, 4 } && end == std::pair{ 7, 2 } ) {
            atLocation( { 7, 3 } ) = { State::WHITE, Pieces::ROOK };
            atLocation( { 7, 0 } ).state = State::EMPTY;
        }
        else if ( start == std::pair{ 7, 4 } && end == std::pair{ 7, 6 } ) {
            atLocation( { 7, 5 } ) = { State::WHITE, Pieces::ROOK };
            atLocation( { 7, 7 } ).state = State::EMPTY;
        }
    }

    // pawn promotion
    if ( endCell.piece == Pieces::PAWN ) {
        if (( endCell.state == State::WHITE && end.first == 0 ) ||
            ( endCell.state == State::BLACK && end.first == 7 ))
            endCell.piece = Pieces::QUEEN;
    }

    // determine if this move placed the other player in check
    {
        MovesDatabase nextTurnMoves{};
        auto          inserter = nextTurnMoves.inserter();
        calculateLegalMoves( inserter, whiteTurn );
        auto kingLocation = whiteTurn ? blackKingLocation : whiteKingLocation;
        nextTurnMoves.db << "SELECT EXISTS(SELECT * FROM moves WHERE end_x = ? AND end_y = ?);"
                         << kingLocation.first << kingLocation.second
                         >> inCheck;
    }

    // setup next turn
    whiteTurn = !whiteTurn;
    moves.db << "DELETE FROM moves";
    auto inserter = moves.inserter();
    calculateLegalMoves( inserter, whiteTurn );

    // determine if this move caused checkmate or caused the other player to have no legal moves (stalemate)
    if ( extendedChecks ) {
        // make a copy of the state of the game and try out each move to see if it keeps the player out of check
        bool  legalMoveExists = false;
        Chess nextTurn{ *this };
        for ( const auto& move: legalMoves()) {
            if ( nextTurn.move( move.start, move.end, false )) {
                legalMoveExists = true;
                break;
            }
        }
        if ( !legalMoveExists ) {
            if ( inCheck ) inCheckmate = true;
            else inStalemate = true;
        }
    }

    // check if the kings are the only pieces left (stalemate)
    {
        bool      pieceExists = false;
        for ( int i           = 0; i < 8; ++i ) {
            for ( int j = 0; j < 8; ++j ) {
                const auto& cell = atLocation( { i, j } );
                if ( cell.state != State::EMPTY && cell.piece != Pieces::KING ) {
                    pieceExists = true;
                    break;
                }
            }
        }
        if ( !pieceExists ) inStalemate = true;
    }

    // update tracking data for castling
    if ( endCell.piece == Pieces::KING ) {
        if ( whiteTurn )
            whiteKingMoved = true;
        else
            blackKingMoved = true;
    }
    else if ( start == std::pair{ 0, 0 } )
        blackQueensRookMoved = true;
    else if ( start == std::pair{ 0, 7 } )
        blackKingsRookMoved = true;
    else if ( start == std::pair{ 7, 0 } )
        whiteQueensRookMoved = true;
    else if ( start == std::pair{ 7, 7 } )
        whiteKingsRookMoved = true;

    return true;
}

void Chess::calculateLegalMoves( MovesDatabase::Inserter& inserter, bool isWhite, bool excludeKing ) {
    for ( int i = 0; i < 8; ++i ) {
        for ( int j = 0; j < 8; ++j ) {
            auto& currentCell = boardState_[ i ][ j ];
            switch ( currentCell.state ) {
                case State::EMPTY:continue;
                case State::WHITE:if ( !isWhite ) continue;
                    break;
                case State::BLACK:if ( isWhite ) continue;
            }

            switch ( currentCell.piece ) {
                case Pieces::PAWN:calculatePawnMoves( inserter, { i, j }, isWhite );
                    break;
                case Pieces::ROOK:calculateRookMoves( inserter, { i, j }, isWhite );
                    break;
                case Pieces::KNIGHT:calculateKnightMoves( inserter, { i, j }, isWhite );
                    break;
                case Pieces::BISHOP:calculateBishopMoves( inserter, { i, j }, isWhite );
                    break;
                case Pieces::QUEEN:calculateQueenMoves( inserter, { i, j }, isWhite );
                    break;
                case Pieces::KING:
                    if ( !excludeKing )
                        calculateKingMoves( inserter, { i, j }, isWhite );
                    break;
            }
        }
    }
}

void Chess::calculatePawnMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite ) {
    if ( isWhite ) {
        std::pair<int, int> oneForward = { location.first - 1, location.second };
        if ( atLocation( oneForward ).state == State::EMPTY )
            inserter.insert( { location, oneForward } );

        // can move two spaces if in starting position
        if ( location.first == 6 ) {
            std::pair<int, int> twoForward = { location.first - 2, location.second };
            if ( atLocation( twoForward ).state == State::EMPTY )
                inserter.insert( { location, twoForward } );
        }

        // check if it can capture
        {
            std::pair<int, int> diagonal = { location.first - 1, location.second - 1 };
            if ( atLocation( diagonal ).state == State::BLACK )
                inserter.insert( { location, diagonal } );
        }
        // check other diagonal
        {
            std::pair<int, int> diagonal = { location.first - 1, location.second + 1 };
            if ( atLocation( diagonal ).state == State::BLACK )
                inserter.insert( { location, diagonal } );
        }
    }
    else {
        std::pair<int, int> oneForward = { location.first + 1, location.second };
        if ( atLocation( oneForward ).state == State::EMPTY )
            inserter.insert( { location, oneForward } );

        // can move two spaces if in starting position
        if ( location.first == 1 ) {
            std::pair<int, int> twoForward = { location.first + 2, location.second };
            if ( atLocation( twoForward ).state == State::EMPTY )
                inserter.insert( { location, twoForward } );
        }

        // check if it can capture
        {
            std::pair<int, int> diagonal = { location.first + 1, location.second - 1 };
            if ( atLocation( diagonal ).state == State::WHITE )
                inserter.insert( { location, diagonal } );
        }
        // check other diagonal
        {
            std::pair<int, int> diagonal = { location.first + 1, location.second + 1 };
            if ( atLocation( diagonal ).state == State::WHITE )
                inserter.insert( { location, diagonal } );
        }
    }
}

namespace {
    bool validateLocation( std::pair<int, int> location ) {
        return location.first >= 0 && location.first < 8 &&
               location.second >= 0 && location.second < 8;
    }
}

void Chess::checkInDirection( MovesDatabase::Inserter& inserter,
                              std::pair<int, int> location,
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
            case State::EMPTY:inserter.insert( { start, location } );
                break;
            case State::WHITE:
                if ( !isWhite )
                    inserter.insert( { start, location } );
                isEmpty = false;
                break;
            case State::BLACK:
                if ( isWhite )
                    inserter.insert( { start, location } );
                isEmpty = false;
                break;
        }
    }
}

void Chess::calculateRookMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite ) {
    checkInDirection( inserter, location, isWhite, []( int& i ) { ++i; }, []( int& ) {} );
    checkInDirection( inserter, location, isWhite, []( int& i ) { --i; }, []( int& ) {} );
    checkInDirection( inserter, location, isWhite, []( int& ) {}, []( int& i ) { ++i; } );
    checkInDirection( inserter, location, isWhite, []( int& ) {}, []( int& i ) { --i; } );
}

void Chess::calculateKnightMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite ) {
    auto sameColor = isWhite ? State::WHITE : State::BLACK;
    {
        std::pair<int, int> destination = { location.first + 2, location.second + 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first + 2, location.second - 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 2, location.second + 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 2, location.second - 1 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first + 1, location.second + 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first + 1, location.second - 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 1, location.second + 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
    {
        std::pair<int, int> destination = { location.first - 1, location.second - 2 };
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    }
}

void Chess::calculateBishopMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite ) {
    checkInDirection( inserter, location, isWhite, []( int& i ) { ++i; }, []( int& i ) { ++i; } );
    checkInDirection( inserter, location, isWhite, []( int& i ) { --i; }, []( int& i ) { ++i; } );
    checkInDirection( inserter, location, isWhite, []( int& i ) { ++i; }, []( int& i ) { --i; } );
    checkInDirection( inserter, location, isWhite, []( int& i ) { --i; }, []( int& i ) { --i; } );
}

void Chess::calculateQueenMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite ) {
    calculateBishopMoves( inserter, location, isWhite );
    calculateRookMoves( inserter, location, isWhite );
}

void Chess::calculateKingMoves( MovesDatabase::Inserter& inserter, std::pair<int, int> location, bool isWhite ) {
    const auto checkDirection = [ & ]( void(* xIncrement)( int& ), void(* yIncrement)( int& )) {
        auto destination = location;
        xIncrement( destination.first );
        yIncrement( destination.second );
        auto sameColor = isWhite ? State::WHITE : State::BLACK;
        if ( validateLocation( destination ) && atLocation( destination ).state != sameColor )
            inserter.insert( { location, destination } );
    };
    checkDirection( []( int& i ) { ++i; }, []( int& ) {} );
    checkDirection( []( int& i ) { --i; }, []( int& ) {} );
    checkDirection( []( int& ) {}, []( int& i ) { ++i; } );
    checkDirection( []( int& ) {}, []( int& i ) { --i; } );
    checkDirection( []( int& i ) { ++i; }, []( int& i ) { ++i; } );
    checkDirection( []( int& i ) { ++i; }, []( int& i ) { --i; } );
    checkDirection( []( int& i ) { --i; }, []( int& i ) { ++i; } );
    checkDirection( []( int& i ) { --i; }, []( int& i ) { --i; } );

    // determine if player can castle
    if ( inCheck ) return;
    if ( isWhite ) {
        if ( whiteKingMoved ) return;
        MovesDatabase otherPlayersMoves{};
        auto          otherInserter = otherPlayersMoves.inserter();
        calculateLegalMoves( otherInserter, !whiteTurn, true );

        if ( !whiteKingsRookMoved &&
             atLocation( { 7, 5 } ).state == State::EMPTY &&
             atLocation( { 7, 6 } ).state == State::EMPTY &&
             atLocation( { 7, 7 } ).state == State::WHITE &&
             atLocation( { 7, 7 } ).piece == Pieces::ROOK
                ) {
            bool castlingThroughCheck;
            otherPlayersMoves.db << "SELECT EXISTS("
                                    "   SELECT * FROM MOVES"
                                    "   WHERE end_x = 7 AND end_y IN (5, 6)"
                                    ");"
                                 >> castlingThroughCheck;
            if ( !castlingThroughCheck )
                inserter.insert( {{ 7, 4 },
                                  { 7, 6 }} );
        }
        if ( !whiteQueensRookMoved &&
             atLocation( { 7, 3 } ).state == State::EMPTY &&
             atLocation( { 7, 2 } ).state == State::EMPTY &&
             atLocation( { 7, 1 } ).state == State::EMPTY &&
             atLocation( { 7, 0 } ).state == State::WHITE &&
             atLocation( { 7, 0 } ).piece == Pieces::ROOK
                ) {
            bool castlingThroughCheck;
            otherPlayersMoves.db << "SELECT EXISTS("
                                    "   SELECT * FROM MOVES"
                                    "   WHERE end_x = 7 AND end_y IN (1,2,3)"
                                    ");"
                                 >> castlingThroughCheck;
            if ( !castlingThroughCheck )
                inserter.insert( {{ 7, 4 },
                                  { 7, 2 }} );
        }
    }
    else {
        if ( blackKingMoved ) return;
        MovesDatabase otherPlayersMoves{};
        auto          otherInserter = otherPlayersMoves.inserter();
        calculateLegalMoves( otherInserter, !whiteTurn, true );

        if ( !blackKingsRookMoved &&
             atLocation( { 0, 5 } ).state == State::EMPTY &&
             atLocation( { 0, 6 } ).state == State::EMPTY &&
             atLocation( { 0, 7 } ).state == State::BLACK &&
             atLocation( { 0, 7 } ).piece == Pieces::ROOK
                ) {
            bool castlingThroughCheck;
            otherPlayersMoves.db << "SELECT EXISTS("
                                    "   SELECT * FROM MOVES"
                                    "   WHERE end_x = 0 AND end_y IN (5, 6)"
                                    ");"
                                 >> castlingThroughCheck;
            if ( !castlingThroughCheck )
                inserter.insert( {{ 0, 4 },
                                  { 0, 6 }} );
        }
        if ( !blackQueensRookMoved &&
             atLocation( { 0, 3 } ).state == State::EMPTY &&
             atLocation( { 0, 2 } ).state == State::EMPTY &&
             atLocation( { 0, 1 } ).state == State::EMPTY &&
             atLocation( { 0, 0 } ).state == State::BLACK &&
             atLocation( { 0, 0 } ).piece == Pieces::ROOK
                ) {
            bool castlingThroughCheck;
            otherPlayersMoves.db << "SELECT EXISTS("
                                    "   SELECT * FROM MOVES"
                                    "   WHERE end_x = 0 AND end_y IN (1,2,3)"
                                    ");"
                                 >> castlingThroughCheck;
            if ( !castlingThroughCheck )
                inserter.insert( {{ 0, 4 },
                                  { 0, 6 }} );
            inserter.insert( {{ 0, 4 },
                              { 0, 2 }} );
        }
    }
}

bool operator==( const Chess::Move& a, const Chess::Move& b ) {
    return a.start == b.start && a.end == b.end;
}

Chess::Cell& Chess::atLocation( std::pair<int, int> location ) {
    return boardState_[ location.first ][ location.second ];
}

Chess::LegalMoves Chess::legalMoves() const {
    LegalMoves legalMoves_;
    moves.db << "SELECT start_x, start_y, end_x, end_y FROM moves;"
             >> [ & ]( int startX, int startY, int endX, int endY ) {
                 legalMoves_.push_back( {{ startX, startY },
                                         { endX,   endY }} );
             };
    return legalMoves_;
}

Chess::MovesDatabase::MovesDatabase() {
    db << "CREATE TABLE moves(start_x INT, start_y INT, end_x INT, end_y INT)";
}

Chess::MovesDatabase::Inserter Chess::MovesDatabase::inserter() {
    return { db << "INSERT INTO moves(start_x, start_y, end_x, end_y) VALUES(?,?,?,?);" };
}

void Chess::MovesDatabase::Inserter::insert( const Chess::Move& move ) {
    binder.reset();
    binder << move.start.first << move.start.second << move.end.first << move.end.second;
    binder.execute();
}
