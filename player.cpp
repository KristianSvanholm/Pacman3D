#include"player.h"

extern vector<glm::vec3> level;
extern bool gameOver;
extern bool win;

Player::Player(glm::vec3 position, float _lastX, float _lastY) {
	pitch = 0;
	cameraPos = position;
	lastX = _lastX, lastY = _lastY;
}

/// <summary>
/// Takes in all legal input from player and handles it.
/// </summary>
/// <param name="window">Window to get input data from</param>
void Player::processInput(GLFWwindow* window, float deltaTime) {
	//Close window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Player movement (Take in direction and ground it so that player cant fly
	glm::vec3 move = cameraFront;
	glm::normalize(move);

	float cameraSpeed = 2.5f * deltaTime;

	//Input handler
	if (!win && !gameOver) { //if game not done
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			movePlayer(move * cameraSpeed);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			movePlayer(-move * cameraSpeed);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			movePlayer(-glm::normalize(glm::cross(move, cameraUp)) * cameraSpeed);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			movePlayer(glm::normalize(glm::cross(move, cameraUp)) * cameraSpeed);
		}
	}
}

/// <summary>
///  Applies new mouse input to camera
/// </summary>
/// <param name="window">GLFW window </param>
/// <param name="xpos"> xpos of mouse on screen </param>
/// <param name="ypos"> ypos of mouse on screen </param>
void Player::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) //Checks if first input and recalibrates to remove screen jump once user clicks screen
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	//Clamp pitch for no funky screen flipping etc
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	//Apply all changes
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

/// <summary>
/// Checks and applies input to player if possible
/// </summary>
/// <param name="input">New input from player controller</param>
void Player::movePlayer(glm::vec3 input) {
	glm::vec3 test = cameraPos;

	//x
	test.x += input.x;
	test.z = cameraPos.z;
	if (!collides(test)) { //If no collision, apply x movement
		cameraPos.x = test.x;
	}
	//z
	test.x = cameraPos.x;
	test.z += input.z;
	if (!collides(test)) { //if no collision, apply y movement
		cameraPos.z = test.z;
	}
}

/// <summary>
/// Checks a new position against all wall segments
/// Returns true if collision, false if no collision
/// </summary>
/// <param name="pos">Proposed new position</param>
/// <returns>If that position collides or not</returns>
bool Player::collides(glm::vec3 pos) {
	bool xColl, zColl;
	float size = 0.75;

	for (int i = 0; i < level.size(); i++) {
		glm::vec3 wall = level[i];

		xColl = wall.x + size >= pos.x && wall.x - size <= pos.x; //X-axis overlap
		zColl = wall.z + size >= pos.z && wall.z - size <= pos.z; //Y-axis overlap

		if (xColl && zColl) { // both overlap :: collision
			return true;
		}
	}
	return false;
}

glm::mat4 Player::generateView() {
	glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	if (!win && !gameOver) {
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
	return view;
}

glm::vec3 Player::getPosition() {
	return cameraPos;
}