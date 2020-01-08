add_subdirectory ( godot-cpp )

add_library ( sqlite STATIC sqlite/sqlite3.c )
target_include_directories ( sqlite PUBLIC sqlite sqlite_modern_cpp/hdr )
