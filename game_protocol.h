#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

#include <vector>
#include <string>

// --- Constants ---
// A simple hardcoded map. ' ' is empty space, '#' is a wall.
const std::vector<std::string> game_map = {
    "########################################",
    "#                                      #",
    "#  ####    ####    ####    ####    ####  #",
    "#  #       #       #  #    #  #    #     #",
    "#  ####    ####    #  #    #  #    ####  #",
    "#     #       #    #  #    #  #       #  #",
    "#  ####    ####    ####    ####    ####  #",
    "#                                      #",
    "#                                      #",
    "#  ##  ##  ##  ##  ##  ##  ##  ##  ##  #",
    "#                                      #",
    "#                                      #",
    "#  ####    ####    ####    ####    ####  #",
    "#  #       #       #  #    #  #    #     #",
    "#  ####    ####    #  #    #  #    ####  #",
    "#     #       #    #  #    #  #       #  #",
    "#  ####    ####    ####    ####    ####  #",
    "#                                      #",
    "#                                      #",
    "########################################"
};

const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 20;
const double GLOBAL_COOLDOWN = 0.1; // 100ms cooldown after each action

// --- Enums ---

// Represents the direction a tank or bullet is facing
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// Represents the actions a client can send to the server
enum class ClientAction : char {
    MOVE_UP = 'W',
    MOVE_DOWN = 'S',
    MOVE_LEFT = 'A',
    MOVE_RIGHT = 'D',
    SHOOT = ' ' // Spacebar to shoot
};


// --- Data Structures ---

// Represents a player's tank
struct Tank {
    int id;
    int x, y;
    Direction dir;
    bool alive;
};

// Represents a bullet
struct Bullet {
    int owner_id;
    int x, y;
    Direction dir;
};

// Represents the entire game state, sent from server to clients
struct GameState {
    std::vector<Tank> tanks;
    std::vector<Bullet> bullets;
    bool game_over = false;
    int winner_id = -1;
};

// --- Serialization ---
// Simple serialization functions to convert structs to strings for network transfer.
// A more robust solution might use JSON or a binary format, but for this
// CLI game, a simple custom format is sufficient.

// Format: "id,x,y,dir,alive;"
inline std::string serialize(const Tank& tank) {
    return std::to_string(tank.id) + "," +
           std::to_string(tank.x) + "," +
           std::to_string(tank.y) + "," +
           std::to_string(static_cast<int>(tank.dir)) + "," +
           std::to_string(tank.alive) + ";";
}

inline Tank deserialize_tank(const std::string& s) {
    Tank tank;
    size_t start = 0;
    size_t end = s.find(',');
    tank.id = std::stoi(s.substr(start, end - start));

    start = end + 1;
    end = s.find(',', start);
    tank.x = std::stoi(s.substr(start, end - start));

    start = end + 1;
    end = s.find(',', start);
    tank.y = std::stoi(s.substr(start, end - start));

    start = end + 1;
    end = s.find(',', start);
    tank.dir = static_cast<Direction>(std::stoi(s.substr(start, end - start)));

    start = end + 1;
    end = s.find(';', start);
    tank.alive = std::stoi(s.substr(start, end - start));

    return tank;
}

// Format: "owner_id,x,y,dir;"
inline std::string serialize(const Bullet& bullet) {
    return std::to_string(bullet.owner_id) + "," +
           std::to_string(bullet.x) + "," +
           std::to_string(bullet.y) + "," +
           std::to_string(static_cast<int>(bullet.dir)) + ";";
}

inline Bullet deserialize_bullet(const std::string& s) {
    Bullet bullet;
    size_t start = 0;
    size_t end = s.find(',');
    bullet.owner_id = std::stoi(s.substr(start, end - start));

    start = end + 1;
    end = s.find(',', start);
    bullet.x = std::stoi(s.substr(start, end - start));

    start = end + 1;
    end = s.find(',', start);
    bullet.y = std::stoi(s.substr(start, end - start));

    start = end + 1;
    end = s.find(';', start);
    bullet.dir = static_cast<Direction>(std::stoi(s.substr(start, end - start)));

    return bullet;
}

// Format: "T[tanks_data]|B[bullets_data]|G[game_over,winner_id]"
inline std::string serialize(const GameState& state) {
    std::string data = "T";
    for (const auto& tank : state.tanks) {
        data += serialize(tank);
    }
    data += "|B";
    for (const auto& bullet : state.bullets) {
        data += serialize(bullet);
    }
    data += "|G";
    data += std::to_string(state.game_over) + "," + std::to_string(state.winner_id);
    return data;
}

inline GameState deserialize_gamestate(const std::string& data) {
    GameState state;

    size_t tanks_start = data.find('T') + 1;
    size_t bullets_start = data.find("|B") + 2;
    size_t game_info_start = data.find("|G") + 2;

    // Deserialize tanks
    std::string tanks_str = data.substr(tanks_start, bullets_start - tanks_start - 2);
    size_t pos = 0;
    while ((pos = tanks_str.find(';')) != std::string::npos) {
        state.tanks.push_back(deserialize_tank(tanks_str.substr(0, pos + 1)));
        tanks_str.erase(0, pos + 1);
    }

    // Deserialize bullets
    std::string bullets_str = data.substr(bullets_start, game_info_start - bullets_start - 2);
    pos = 0;
    while ((pos = bullets_str.find(';')) != std::string::npos) {
        state.bullets.push_back(deserialize_bullet(bullets_str.substr(0, pos + 1)));
        bullets_str.erase(0, pos + 1);
    }

    // Deserialize game info
    std::string game_info_str = data.substr(game_info_start);
    pos = game_info_str.find(',');
    state.game_over = std::stoi(game_info_str.substr(0, pos));
    state.winner_id = std::stoi(game_info_str.substr(pos + 1));

    return state;
}

#endif // GAME_PROTOCOL_H