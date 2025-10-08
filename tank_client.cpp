#include "tcp_socket.h"
#include "game_protocol.h"
#include <ncurses.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>

// --- Global Variables ---
std::mutex ncurses_mutex; // To safely update the screen from the rendering thread
std::atomic<bool> client_running(true);
GameState current_game_state;
std::mutex game_state_mutex;

// --- Function Prototypes ---
void setup_ncurses();
void cleanup_ncurses();
void rendering_thread(TCPSocket& socket);
void draw_game();
void draw_tank(const Tank& tank);
void draw_bullet(const Bullet& bullet);

// --- Main Function ---
int main(int argc, char *argv[]) {
    std::string host = "127.0.0.1";
    if (argc > 1) {
        host = argv[1];
    }

    // Connect to the server
    TCPSocket client_socket;
    if (!client_socket.connect(host, 8080)) {
        std::cerr << "Failed to connect to server at " << host << ":8080" << std::endl;
        return 1;
    }
    std::cout << "Connected to server." << std::endl;

    // Setup ncurses
    setup_ncurses();

    // Start rendering thread
    std::thread renderer(rendering_thread, std::ref(client_socket));

    // --- Input Loop ---
    int ch;
    while (client_running && (ch = getch()) != 'q') {
        ClientAction action;
        bool should_send = true;
        switch (ch) {
            case 'w':
            case 'W':
                action = ClientAction::MOVE_UP;
                break;
            case 's':
            case 'S':
                action = ClientAction::MOVE_DOWN;
                break;
            case 'a':
            case 'A':
                action = ClientAction::MOVE_LEFT;
                break;
            case 'd':
            case 'D':
                action = ClientAction::MOVE_RIGHT;
                break;
            case ' ':
                action = ClientAction::SHOOT;
                break;
            default:
                should_send = false;
                break;
        }

        if (should_send) {
            std::string action_str(1, static_cast<char>(action));
            client_socket.send(action_str);
        }
    }

    // Cleanup
    client_running = false;
    if (renderer.joinable()) {
        renderer.join();
    }
    cleanup_ncurses();
    client_socket.close();

    return 0;
}

// --- ncurses Functions ---
void setup_ncurses() {
    initscr();            // Start curses mode
    cbreak();             // Line buffering disabled
    noecho();             // Don't echo() while we do getch
    curs_set(0);          // Hide the cursor
    nodelay(stdscr, TRUE); // Make getch non-blocking
    keypad(stdscr, TRUE); // Enable function keys
    start_color();        // Enable color
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Player Tank
    init_pair(2, COLOR_RED, COLOR_BLACK);    // Enemy Tank
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Bullet
    init_pair(4, COLOR_WHITE, COLOR_BLACK);  // Wall
    init_pair(5, COLOR_CYAN, COLOR_BLACK);   // Game Over Text
}

void cleanup_ncurses() {
    endwin();
}

// --- Threading Functions ---
void rendering_thread(TCPSocket& socket) {
    while (client_running) {
        std::string data = socket.recv(4096); // Use a larger buffer
        if (data.empty()) {
            // Server disconnected
            client_running = false;
            break;
        }

        {
            std::lock_guard<std::mutex> lock(game_state_mutex);
            current_game_state = deserialize_gamestate(data);
        }

        draw_game();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

// --- Drawing Functions ---
void draw_game() {
    std::lock_guard<std::mutex> lock(ncurses_mutex);
    erase(); // Clear the screen

    // Draw map
    attron(COLOR_PAIR(4));
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (game_map[y][x] == '#') {
                mvaddch(y, x, '#');
            }
        }
    }
    attroff(COLOR_PAIR(4));

    // Draw tanks and bullets
    {
        std::lock_guard<std::mutex> gs_lock(game_state_mutex);
        for (const auto& tank : current_game_state.tanks) {
            if (tank.alive) {
                draw_tank(tank);
            }
        }
        for (const auto& bullet : current_game_state.bullets) {
            draw_bullet(bullet);
        }

        // Draw Game Over message
        if (current_game_state.game_over) {
            attron(COLOR_PAIR(5) | A_BOLD);
            attron(COLOR_PAIR(5) | A_BOLD);
            std::string msg = "GAME OVER!";
            mvprintw(MAP_HEIGHT / 2 - 1, (MAP_WIDTH - msg.length()) / 2, "%s", msg.c_str());

            if (current_game_state.winner_id != -1) {
                msg = "Winner is Player #" + std::to_string(current_game_state.winner_id);
            } else {
                msg = "It's a draw!";
            }
            mvprintw(MAP_HEIGHT / 2, (MAP_WIDTH - msg.length()) / 2, "%s", msg.c_str());
            attroff(COLOR_PAIR(5) | A_BOLD);
            attroff(COLOR_PAIR(5) | A_BOLD);
        }
    }

    refresh(); // Update the physical screen
}

void draw_tank(const Tank& tank) {
    char symbol;
    switch (tank.dir) {
        case Direction::UP:    symbol = '^'; break;
        case Direction::DOWN:  symbol = 'v'; break;
        case Direction::LEFT:  symbol = '<'; break;
        case Direction::RIGHT: symbol = '>'; break;
        default:               symbol = '?'; break;
    }
    // For now, all tanks are green. Could be changed to distinguish player vs enemy.
    attron(COLOR_PAIR(1) | A_BOLD);
    mvaddch(tank.y, tank.x, symbol);
    attroff(COLOR_PAIR(1) | A_BOLD);
}

void draw_bullet(const Bullet& bullet) {
    attron(COLOR_PAIR(3));
    mvaddch(bullet.y, bullet.x, '*');
    attroff(COLOR_PAIR(3));
}