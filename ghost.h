#ifndef Ghost_header
#define Ghost_header
#include<vector>
#include "glm/glm/glm.hpp"
using namespace std;

class Ghost {
private:
    void Ghost::printMap();
    std::vector<std::vector<int>> level;
    bool transform;
    glm::vec2 dir;
    int currentDir;
    bool turn = false;
    glm::vec3 gridPosition;
    glm::vec3 prevGridPosition;
    glm::vec3 exactPosition;
    float linTime = 0;
public:
    float lastTime = 0;
    void Ghost::updateGhost(float time, float dt);
    Ghost(std::vector<std::vector<int>> _level, int _x, int _y);
    glm::vec3 getPosition();
    bool checkDir(int _x, int _y);
    void lerp(float dt);
    void move(float time);
    int newDirection();
};

#endif