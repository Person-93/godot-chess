add_library ( chess SHARED
              ChessLibrary.cpp
              Chess.hpp
              Chess.cpp
              ChessWrapper.hpp
              ChessWrapper.cpp
              )
target_link_libraries ( chess godot-cpp sqlite )
godot_target ( chess ${CMAKE_SOURCE_DIR}/godot )
