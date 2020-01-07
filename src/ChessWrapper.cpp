#include "ChessWrapper.hpp"

using namespace godot;

void ChessWrapper::_register_methods() {
    register_method( "move", &ChessWrapper::move );
    register_method( "board_state", &ChessWrapper::boardState );
    register_method( "is_white_turn", &ChessWrapper::isWhiteTurn );
    register_method( "is_in_check", &ChessWrapper::isInCheck );
}

void ChessWrapper::_init() {
    boardState_.resize( 64 );
    convertBoardState();
    Godot::print( String( "Legal moves: " ) + Variant(chess.legalMoves().size()));
}

namespace {
    String convertCell( const Chess::Cell& cell ) {
        String string;
        switch ( cell.state ) {
            case Chess::State::EMPTY:return "empty";
            case Chess::State::WHITE:string = "white ";
                break;
            case Chess::State::BLACK:string = "black ";
                break;
        }
        switch ( cell.piece ) {
            case Chess::Pieces::PAWN:string += "pawn";
                break;
            case Chess::Pieces::ROOK:string += "rook";
                break;
            case Chess::Pieces::KNIGHT:string += "knight";
                break;
            case Chess::Pieces::BISHOP:string += "bishop";
                break;
            case Chess::Pieces::QUEEN:string += "queen";
                break;
            case Chess::Pieces::KING:string += "king";
                break;
        }
        return string;
    }
}

bool ChessWrapper::move( godot::Vector2 start, godot::Vector2 end ) {
    Godot::print( String( "Moving from " ) + start + " to " + end );
    bool result = chess.move( { start.x, start.y }, { end.x, end.y } );
    if ( result ) {
        boardState_.set((int) ( end.x * 8 + end.y ), convertCell( chess.boardState()[ end.x ][ end.y ] ));
        boardState_.set((int) ( start.x * 8 + start.y ), "empty" );
        Godot::print( String( "Legal moves: " ) + chess.legalMoves().size());
    }
    else {
        Godot::print( "Illegal move!" );
    }
    return result;
}

void ChessWrapper::convertBoardState() {
    for ( int i = 0; i < 8; ++i ) {
        for ( int j = 0; j < 8; ++j ) {
            boardState_.set( i * 8 + j, convertCell( chess.boardState()[ i ][ j ] ));
        }
    }
}
