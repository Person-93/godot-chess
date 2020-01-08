#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"
#pragma once

#include <Godot.hpp>
#include <Node2D.hpp>
#include "Chess.hpp"

class ChessWrapper : public godot::Node2D {
GODOT_CLASS( ChessWrapper, Node2D )
public:
    static void _register_methods();

    void _init();

    [[nodiscard]] const godot::PoolStringArray& boardState() const { return boardState_; }

    bool move( godot::Vector2 start, godot::Vector2 end );

    [[nodiscard]] bool isWhiteTurn() const { return chess.isWhiteTurn(); }

    [[nodiscard]] bool isInCheck() const { return chess.isInCheck(); }

    [[nodiscard]] bool isInCheckmate() const { return chess.isInCheckmate(); }

private:
    void convertBoardState();

    Chess                  chess;
    godot::PoolStringArray boardState_;
};

#pragma clang diagnostic pop
