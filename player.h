#ifndef Player_header
#define Player_header

#include <GLFW/glfw3.h>
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include <vector>

using namespace std;

class Player{
private:
	//Camera variables
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float yaw = -90.0f;
	float pitch, lastX, lastY;
	bool firstMouse = true;

	//functions
	void movePlayer(glm::vec3 input);
	bool collides(glm::vec3 pos);
public:
	Player(glm::vec3 pos, float _lastX, float _lastY);
	void processInput(GLFWwindow* window, float deltaTime);
	void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	glm::mat4 Player::generateView();
	glm::vec3 Player::getPosition();
};
#endif