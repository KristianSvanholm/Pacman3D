#ifndef Ghost_header
#define Ghost_header

#include <vector>
#include "glm/glm/glm.hpp"

using namespace std;

class Ghost {
private:
    std::vector<std::vector<int>> level;
    glm::vec3 prevGridPosition;
    glm::vec3 exactPosition;
    glm::vec3 gridPosition;
    glm::vec2 dir;
    float linTime = 0;
    bool transform;
    bool turn = false;
    int currentDir;
public:
    Ghost(std::vector<std::vector<int>> _level, int _x, int _y);
    void Ghost::updateGhost(float dt);
    bool checkDir(int _x, int _y);
    glm::vec3 getPosition();
    void lerp(float dt);
    float lastTime = 0;
    int newDirection();
    void move();
};

#endif