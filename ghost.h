#ifndef Ghost_header
#define Ghost_header

#include <vector>
#include <iostream>
#include "glm/glm/glm.hpp"

using namespace std;

class Ghost {
private:
    //Variables
    std::vector<std::vector<int>> level;
    glm::vec3 prevGridPosition;
    glm::vec3 exactPosition;
    glm::vec3 gridPosition;
    glm::vec2 dir;
    float linTime = 0;
    bool transform;
    bool turn = false;
    int currentDir;

    //Functions
    int newDirection();
    bool checkDir(int _x, int _y);
    void lerp(float dt);
    void move();
public:
    Ghost(std::vector<std::vector<int>> _level, int _x, int _y);
    glm::vec3 updateGhost(float dt);
};

#endif