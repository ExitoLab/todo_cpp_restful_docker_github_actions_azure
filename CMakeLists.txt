cmake_minimum_required(VERSION 3.10)
project(todo_cpp)

set(CMAKE_CXX_STANDARD 17)

# Find crow
find_package(crow REQUIRED)

# Find MongoDB C++ Driver
find_package(BSONCXX REQUIRED)
find_package(MONGOCXX REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Add executable
add_executable(todo_cpp main.cpp)

# Link libraries
target_link_libraries(todo_cpp PRIVATE
    crow::crow
    BSONCXX::bsoncxx
    MONGOCXX::mongocxx
    Boost::system
)

# Set rpath to find libraries within the docker image
set_target_properties(todo_cpp PROPERTIES
    BUILD_RPATH "\$ORIGIN/../lib"
    INSTALL_RPATH "\$ORIGIN/../lib"
)