#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <functional>
#include <stdexcept>
#include <queue>
#include <unordered_map>
#include <chrono>

struct vec3 {
    int x, y, z;

    int get_index(int sx, int sy, int sz) const {
        return x * sy * sz + y * sz + z;
    }

    bool operator==(const vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    vec3 operator+(const vec3& other) const {
        return{x + other.x, y + other.y, z + other.z};
    }

    static vec3 min(const vec3& a, const vec3& b) {
        return{std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
    }

    static vec3 max(const vec3& a, const vec3& b) {
        return{std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
    }

    static float dist(const vec3& a, const vec3& b) {
        auto dx = static_cast<float>(a.x - b.x);
        auto dy = static_cast<float>(a.y - b.y);
        auto dz = static_cast<float>(a.z - b.z);

        //return sqrtf(dx*dx + dy*dy + dz*dz); // no need to sqrt as this is only a metric to compare path  
        return (dx*dx + dy*dy + dz*dz); // <- increase speed
    }
};

namespace std {
    template <>
    struct hash<vec3> {
        size_t operator()(const vec3& k) const {
            return ((hash<int>()(k.x)
                ^ (hash<int>()(k.y) << 1)) >> 1)
                ^ (hash<int>()(k.z) << 1);
        }
    };
}

struct cell {
    bool occupied;
    bool walkableSurface;
};

int sx, sy, sz;
std::vector<cell> grid;

std::vector<vec3> get_neighbours(const vec3& cell) {
    std::vector<vec3> neighbours;

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                auto coord = cell + vec3{dx, dy, dz};

                bool notSelf = !(dx == 0 && dy == 0 && dz == 0);
                bool connectivity = abs(dx) + abs(dy) + abs(dz) <= 2;
                bool withinGrid = coord.x >= 0 && coord.y >= 0 && coord.z >= 0 && coord.x < sx && coord.y < sy && coord.z < sz;

                if (notSelf && connectivity && withinGrid) {
                    neighbours.push_back(coord);
                }
            }
        }
    }

    return neighbours;
}

std::vector<vec3> find_path(const vec3& start, const vec3& end, bool(*cellFilter)(const vec3&, const vec3&)) {
    if (!cellFilter(start, start) || !cellFilter(end, end)) {
        throw std::invalid_argument("start and/or end fail cell filter!");
    }

    // Initialize data structures
    std::unordered_map<vec3, float> dist;
    std::unordered_map<vec3, vec3> prev;

    struct queue_node {
        vec3 value;
        float dist;
    };

    auto cmp = [&](const queue_node& a, const queue_node& b) {
        return a.dist > b.dist;
    };

    std::priority_queue<queue_node, std::vector<queue_node>, decltype(cmp)> Q(cmp);

    for (int x = 0; x < sx; x++) {
        for (int y = 0; y < sy; y++) {
            for (int z = 0; z < sz; z++) {
                vec3 coord = {x, y, z};

                if (cellFilter(coord, coord)) {
                    dist[coord] = std::numeric_limits<float>::max();
                    Q.push({coord, std::numeric_limits<float>::max()});

                    prev[coord] = vec3{-1, -1, -1};
                }
            }
        }
    }

    dist[start] = 0;
    Q.push({start, 0});

    // Search loop
    while (!Q.empty()) {
        auto u = Q.top();
        Q.pop();

        // Old priority queue value
        if (u.dist != dist[u.value]) {
            continue;
        }

        if (u.value == end) {
            break;
        }

        for (const vec3& v : get_neighbours(u.value)) {
            if (cellFilter(u.value, v)) {
                float alt = dist[u.value] + vec3::dist(u.value, v);
                if (alt < dist[v]) {
                    dist[v] = alt;
                    Q.push({v, alt});

                    prev[v] = u.value;
                }
            }
        }
    }

    // Trace path - if there is one
    std::vector<vec3> path;

    if (prev[end].x != -1) {
        vec3 current = end;

        while (current.x != -1) {
            path.push_back(current);
            current = prev[current];
        }

        std::reverse(path.begin(), path.end());
    }

    return path;
}

bool isFloor(const vec3& pos) {
    if (pos.y > 0) {
        return !grid[pos.get_index(sx, sy, sz)].occupied && grid[(pos + vec3{0, -1, 0}).get_index(sx, sy, sz)].walkableSurface;
    } else {
        return false;
    }
}

bool cellFilter(const vec3& from, const vec3& to) {
    if (from.y == to.y) {
        // Check if all cells we're moving through are floors (important when moving diagonally)
        auto min = vec3::min(from, to);
        auto max = vec3::max(from, to);

        for (int x = min.x; x <= max.x; x++) {
            for (int z = min.z; z <= max.z; z++) {
                if (!isFloor({x, min.y, z})) {
                    return false;
                }
            }
        }

        return true;
    } else {
        // If the movement is vertical, then perform no diagonal check
        return isFloor(to);
    }
}
