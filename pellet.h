#ifndef pellet_header
#define pellet_header
#include "glm/glm/glm.hpp"

class Pellet {
public:
    Pellet(glm::vec3 _position);
    glm::vec3 position;
    bool active;
};

#endif