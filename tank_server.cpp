#include "tcp_socket.h"
#include "game_protocol.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>
#include <queue>
#include <unordered_map>
#include <memory> // Required for std::shared_ptr
#include <algorithm> // Required for std::find_if, std::remove_if

// --- Global Variables ---
std::mutex game_state_mutex;
GameState game_state;

std::mutex clients_mutex;
std::unordered_map<int, std::shared_ptr<TCPSocket>> clients;

std::mutex action_queue_mutex;
std::queue<std::pair<int, ClientAction>> action_queue;

std::atomic<bool> server_running(true);
std::atomic<int> next_player_id(1);

// --- Function Prototypes ---
void game_loop();
void handle_client(std::shared_ptr<TCPSocket> client_socket, int player_id);
void broadcast_state();
Tank create_new_player(int player_id);
bool is_wall(int x, int y);

// --- Main Function ---
int main() {
    srand(time(0)); // Seed for random player positions

    // Start the game loop in a separate thread
    std::thread game_thread(game_loop);

    // Setup the server
    TCPSocket server(8080, "0.0.0.0");
    if (!server.is_valid() || !server.listen(5)) {
        std::cerr << "Failed to start server." << std::endl;
        server_running = false;
        if(game_thread.joinable()) game_thread.join();
        return 1;
    }

    std::cout << "Server listening on port 8080..." << std::endl;

    // Accept new clients
    while (server_running) {
        auto client_socket_unique = server.accept();
        if (client_socket_unique) {
            // Convert unique_ptr to shared_ptr for shared ownership
            std::shared_ptr<TCPSocket> client_socket_shared = std::move(client_socket_unique);

            int player_id = next_player_id++;
            std::cout << "Player #" << player_id << " connected." << std::endl;

            // Add client to the shared map
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients[player_id] = client_socket_shared;
            }

            // Create a new tank for the player
            {
                std::lock_guard<std::mutex> lock(game_state_mutex);
                game_state.tanks.push_back(create_new_player(player_id));
            }

            // Start a thread to handle this client, passing the shared_ptr
            std::thread client_thread(handle_client, client_socket_shared, player_id);
            client_thread.detach();
        }
    }

    if (game_thread.joinable()) {
        game_thread.join();
    }

    return 0;
}

// --- Function Implementations ---

// The main loop that runs the game logic
void game_loop() {
    auto last_update_time = std::chrono::steady_clock::now();
    auto last_action_time = std::chrono::steady_clock::now();

    while (server_running) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_since_update = now - last_update_time;

        // --- Process Actions Queue---
        std::chrono::duration<double> elapsed_since_action = now - last_action_time;
        if (elapsed_since_action.count() > GLOBAL_COOLDOWN) {
            std::lock_guard<std::mutex> lock(action_queue_mutex);
            if (!action_queue.empty()) {
                auto action = action_queue.front();
                action_queue.pop();
                int player_id = action.first;
                ClientAction client_action = action.second;

                // Process the action
                std::lock_guard<std::mutex> gs_lock(game_state_mutex);
                auto it = std::find_if(game_state.tanks.begin(), game_state.tanks.end(),
                                       [player_id](const Tank& t){ return t.id == player_id; });

                if (it != game_state.tanks.end() && it->alive) {
                    Tank& tank = *it;
                    int next_x = tank.x;
                    int next_y = tank.y;

                    switch (client_action) {
                        case ClientAction::MOVE_UP: tank.dir = Direction::UP; next_y--; break;
                        case ClientAction::MOVE_DOWN: tank.dir = Direction::DOWN; next_y++; break;
                        case ClientAction::MOVE_LEFT: tank.dir = Direction::LEFT; next_x--; break;
                        case ClientAction::MOVE_RIGHT: tank.dir = Direction::RIGHT; next_x++; break;
                        case ClientAction::SHOOT:
                            {
                                Bullet bullet;
                                bullet.owner_id = tank.id;
                                bullet.dir = tank.dir;
                                switch (tank.dir) { // Bullet appears in front of tank
                                    case Direction::UP: bullet.x = tank.x; bullet.y = tank.y - 1; break;
                                    case Direction::DOWN: bullet.x = tank.x; bullet.y = tank.y + 1; break;
                                    case Direction::LEFT: bullet.x = tank.x - 1; bullet.y = tank.y; break;
                                    case Direction::RIGHT: bullet.x = tank.x + 1; bullet.y = tank.y; break;
                                }
                                if (!is_wall(bullet.x, bullet.y)) {
                                    game_state.bullets.push_back(bullet);
                                }
                            }
                            break;
                    }

                    if (client_action != ClientAction::SHOOT && !is_wall(next_x, next_y)) {
                        tank.x = next_x;
                        tank.y = next_y;
                    }
                }
                last_action_time = now;
            }
        }

        // --- Physics and Game Logic Update (runs at a fixed rate) ---
        if (elapsed_since_update.count() >= 0.05) { // 20 times per second
            last_update_time = now;

            // --- Update Bullets ---
            {
                std::lock_guard<std::mutex> lock(game_state_mutex);
                for (auto& bullet : game_state.bullets) {
                    switch (bullet.dir) {
                        case Direction::UP: bullet.y--; break;
                        case Direction::DOWN: bullet.y++; break;
                        case Direction::LEFT: bullet.x--; break;
                        case Direction::RIGHT: bullet.x++; break;
                    }
                }
                // Remove bullets that hit walls
                game_state.bullets.erase(std::remove_if(game_state.bullets.begin(), game_state.bullets.end(),
                    [](const Bullet& b){ return is_wall(b.x, b.y); }), game_state.bullets.end());
            }

            // --- Check Collisions ---
            {
                std::lock_guard<std::mutex> lock(game_state_mutex);
                std::vector<Bullet*> bullets_to_remove;
                for (auto& tank : game_state.tanks) {
                    if (!tank.alive) continue;
                    for (auto& bullet : game_state.bullets) {
                        if (tank.id != bullet.owner_id && tank.x == bullet.x && tank.y == bullet.y) {
                            tank.alive = false;
                            bullets_to_remove.push_back(&bullet);
                        }
                    }
                }
                // Remove bullets that hit tanks
                game_state.bullets.erase(std::remove_if(game_state.bullets.begin(), game_state.bullets.end(),
                    [&](const Bullet& b){
                        for(auto* ptr : bullets_to_remove) {
                            if (ptr == &b) return true;
                        }
                        return false;
                    }), game_state.bullets.end());
            }

            // --- Check Win Condition ---
            {
                std::lock_guard<std::mutex> lock(game_state_mutex);
                if (!game_state.game_over && game_state.tanks.size() >= 1) {
                    int alive_count = 0;
                    int winner_id = -1;
                    for(const auto& tank : game_state.tanks) {
                        if(tank.alive) {
                            alive_count++;
                            winner_id = tank.id;
                        }
                    }
                    if(alive_count <= 1) {
                        game_state.game_over = true;
                        game_state.winner_id = winner_id;
                        std::cout << "Game Over! Winner is Player #" << winner_id << std::endl;
                    }
                }
            }

            // --- Broadcast State to all clients ---
            broadcast_state();
        }

        // Sleep to avoid busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// Handles receiving actions from a single client
void handle_client(std::shared_ptr<TCPSocket> client_socket, int player_id) {
    while (server_running && client_socket->is_valid()) {
        std::string data = client_socket->recv(1); // Receive one character at a time
        if (data.empty()) {
            break; // Client disconnected
        }

        ClientAction action = static_cast<ClientAction>(data[0]);
        {
            std::lock_guard<std::mutex> lock(action_queue_mutex);
            action_queue.push({player_id, action});
        }
    }

    // Cleanup on disconnect
    std::cout << "Player #" << player_id << " disconnected." << std::endl;
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(player_id);
    }
    {
        std::lock_guard<std::mutex> lock(game_state_mutex);
        game_state.tanks.erase(std::remove_if(game_state.tanks.begin(), game_state.tanks.end(),
            [player_id](const Tank& t){ return t.id == player_id; }), game_state.tanks.end());
    }
}

// Sends the current game state to all connected clients
void broadcast_state() {
    std::string serialized_state;
    {
        std::lock_guard<std::mutex> lock(game_state_mutex);
        serialized_state = serialize(game_state);
    }

    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto const& [id, socket_ptr] : clients) {
        if (socket_ptr && socket_ptr->is_valid()) {
            socket_ptr->send(serialized_state);
        }
    }
}

// Creates a new player tank at a random valid location
Tank create_new_player(int player_id) {
    Tank tank;
    tank.id = player_id;
    tank.alive = true;
    tank.dir = Direction::UP;

    // Find a random empty spot to spawn
    while (true) {
        int x = 1 + (rand() % (MAP_WIDTH - 2));
        int y = 1 + (rand() % (MAP_HEIGHT - 2));
        if (!is_wall(x, y)) {
            // Check if the spot is occupied by another tank
            bool occupied = false;
            std::lock_guard<std::mutex> lock(game_state_mutex);
            for(const auto& other_tank : game_state.tanks) {
                if (other_tank.x == x && other_tank.y == y) {
                    occupied = true;
                    break;
                }
            }
            if (!occupied) {
                tank.x = x;
                tank.y = y;
                break;
            }
        }
    }
    return tank;
}

// Checks if a coordinate is a wall
bool is_wall(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return true; // Out of bounds is a wall
    }
    return game_map[y][x] == '#';
}