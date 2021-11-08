#include "pellet.h"

Pellet::Pellet(glm::vec3 _position) {
	position = _position;
	position.y = -0.5f;
	active = true;
}