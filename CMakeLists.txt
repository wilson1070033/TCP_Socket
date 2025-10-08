cmake_minimum_required(VERSION 3.10)
project(TankGame)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread") # Add pthread flag for std::thread

# --- Executables ---

# Server
add_executable(tank_server
    tank_server.cpp
    tcp_socket.cpp
)
# On Linux, threading is handled by pthread, which is enabled by the CXX_FLAG.
# No extra linking is typically needed for basic sockets, but explicit is better if required.

# Client
add_executable(tank_client
    tank_client.cpp
    tcp_socket.cpp
)
# Link ncurses library to the client
target_link_libraries(tank_client PRIVATE ncurses)

# Set output directory for executables
set_target_properties(tank_server tank_client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)