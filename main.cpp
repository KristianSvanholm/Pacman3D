//Standard libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Glad & GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//GLM
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

//Learn OPGENL
#include "learnopengl/shader_m.h"
#include "learnopengl/filesystem.h"

//custom classes
#include "pellet.h"

using namespace std;

//Methods
void readLevel(string path);
unsigned int initializeTexture(string path);
GLuint wallSegment();
GLuint pellet();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
int initialize();
//------

//Game variables
vector<glm::vec3> level;
vector<glm::vec3> pellets;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool win = false;
bool gameOver = false;
//-------------

//Screen
const float WIDTH = 1000;
const float HEIGHT = 800;
const float ASPECT = WIDTH / HEIGHT;   // desired aspect ratio
GLFWwindow* window;
//-----

//Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch;
float lastX = WIDTH/2, lastY = HEIGHT/2;
bool firstMouse = true;
// --------------

int main() {
	readLevel("../../../levels/level0");

	if (initialize() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	Shader ourShader("../../../shaders/7.1.camera.vs", "../../../shaders/7.1.camera.xs");

	// load and create a texture from path
	unsigned int wallTexture = initializeTexture("../../../../resources/textures/wall.jpg");
	unsigned int pelletTexture = initializeTexture("../../../../resources/textures/yellow.jpg");

	GLuint wallVAO = wallSegment();
	GLuint pelletVAO = pellet();

	//Gluint pellets = createPelletVao(); -> This should call createSphere();

	//Create Ghost vao
	//GLuint Ghost = Load3DModel(path);

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.use();
	ourShader.setInt("texture", 0);

	// pass projection matrix to shader (as projection matrix rarely changes there's no need to do this per frame)
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	ourShader.setMat4("projection", projection);

	//Input configuration && callback method
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	//Main game loop
	while(!glfwWindowShouldClose(window)){

		//Deltatime calculation
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//PELLETS
		for (int i = 0; i < pellets.size(); i++) {
			//If pellets withing pickup range of player: remove it from vector
			if (glm::distance(pellets[i], cameraPos) < 0.5f) {
				pellets.erase(pellets.begin()+i);
			}
		}
		if (pellets.size() == 0) { //win condition
			win = true;
		}

		//userInput
		processInput(window);

		//Run ghost AIs

		//Draw everything \/\/\/
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader
		ourShader.use();

		// camera/view transformation
		glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		if (!win && !gameOver) {
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		}
		ourShader.setMat4("view", view);

		//Draw walls && bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		glBindVertexArray(wallVAO);
		for (unsigned int i = 0; i < level.size(); i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, level[i]);
			ourShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Draw pellets && bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pelletTexture);

		glBindVertexArray(pelletVAO);
		for (unsigned int i = 0; i < pellets.size(); i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, pellets[i]);
			ourShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Draw ghost(s)

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Termination of Stuff 
	//TODO::cleanVAO(); on all vaos
	glfwTerminate();
}

/// <summary>
///  Mouse controller for first person view
/// </summary>
/// <param name="window">GLFW window </param>
/// <param name="xpos"> xpos of mouse on screen </param>
/// <param name="ypos"> ypos of mouse on screen </param>
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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
/// Loads texture from path and returns it
/// </summary>
/// <param name="path"> path to texture</param>
/// <returns>texture</returns>
unsigned int initializeTexture(string path) {
	unsigned int texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(FileSystem::getPath(path).c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture from " << path << std::endl;
	}
	stbi_image_free(data);
	return texture;
}

//TODO:: Replace with a key callback
void processInput(GLFWwindow* window)
{
	//Close window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Player movement (Take in direction and ground it so that player cant fly
	glm::vec3 move = cameraFront;
	move.y = 0;
	glm::normalize(move);

	float cameraSpeed = 2.5f * deltaTime;	

	//Input handler
	if (!win && !gameOver) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * move;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * move;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(move, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(move, cameraUp)) * cameraSpeed;
	}
}

/// <summary>
/// GLFW and GLAD initialization with error handling
/// </summary>
/// <returns> success code. Either 0 for success or 1 for failure</returns>
int initialize() {
	//Initialises GLFW
	if (!glfwInit()) {
		cerr << "GLFW Failed initializing. \n";
		cin.get();
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Hints for Window (version & profile)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Creates the Window
	window = glfwCreateWindow(WIDTH, HEIGHT, "Pacman3D", NULL, NULL);

	// Error check for Window
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Sets OpenGL context
	glfwMakeContextCurrent(window);

	//Initialises GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	//Nothing went wrong
	return 0;
}

void readLevel(string path) {
	ifstream lvlFile(path);
	if (lvlFile)
	{
		string size;
		lvlFile >> size;
		int xMax = stoi(size.substr(0, 2));
		int yMax = stoi(size.substr(3));

		// Print current level
		cout << xMax << "*" << yMax << endl;
		for (int i = 0; i < yMax; i++) {
			for (int j = 0; j < xMax; j++) {
				int data;
				lvlFile >> data;
				if (data == 1) {
					level.push_back(glm::vec3(i, 0, j));
				}
				else if (data == 0) {
					pellets.push_back(glm::vec3(i, -0.25, j));
				}
				else if (data == 2) {
					cameraPos.x = i;
					cameraPos.z = j;
				}

			}
			cout << endl;
		}

		//TODO:: REWRITE THIS
		//Generate ghost position
		//RNG seeded by current time in seconds since January 1st, 1970
		srand(time(NULL));
		int randX, randY;
		do {
			randX = rand() % (xMax - 1);
			randY = rand() % (yMax - 1);
		} while (false); // checks for tunnel
		//TODO Instantiate new ghost(s?) at given position(s?)
		//ghost = new Ghost(randX, randY);

	}
	else {
		cout << "\n --Unable to read file " << path;
	}
	lvlFile.close();
}

GLuint wallSegment() {
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 //Removed top and bottom faces as they will never be seen anyways
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	return VAO;
}

GLuint pellet() {
	float vertices[] = {
		-0.10f, -0.10f, -0.10f,  0.0f, 0.0f,
		 0.10f, -0.10f, -0.10f,  1.0f, 0.0f,
		 0.10f,  0.10f, -0.10f,  1.0f, 1.0f,
		 0.10f,  0.10f, -0.10f,  1.0f, 1.0f,
		-0.10f,  0.10f, -0.10f,  0.0f, 1.0f,
		-0.10f, -0.10f, -0.10f,  0.0f, 0.0f,

		-0.10f, -0.10f,  0.10f,  0.0f, 0.0f,
		 0.10f, -0.10f,  0.10f,  1.0f, 0.0f,
		 0.10f,  0.10f,  0.10f,  1.0f, 1.0f,
		 0.10f,  0.10f,  0.10f,  1.0f, 1.0f,
		-0.10f,  0.10f,  0.10f,  0.0f, 1.0f,
		-0.10f, -0.10f,  0.10f,  0.0f, 0.0f,

		-0.10f,  0.10f,  0.10f,  1.0f, 0.0f,
		-0.10f,  0.10f, -0.10f,  1.0f, 1.0f,
		-0.10f, -0.10f, -0.10f,  0.0f, 1.0f,
		-0.10f, -0.10f, -0.10f,  0.0f, 1.0f,
		-0.10f, -0.10f,  0.10f,  0.0f, 0.0f,
		-0.10f,  0.10f,  0.10f,  1.0f, 0.0f,

		 0.10f,  0.10f,  0.10f,  1.0f, 0.0f,
		 0.10f,  0.10f, -0.10f,  1.0f, 1.0f,
		 0.10f, -0.10f, -0.10f,  0.0f, 1.0f,
		 0.10f, -0.10f, -0.10f,  0.0f, 1.0f,
		 0.10f, -0.10f,  0.10f,  0.0f, 0.0f,
		 0.10f,  0.10f,  0.10f,  1.0f, 0.0f,

		-0.10f,  0.10f, -0.10f,  0.0f, 1.0f,
		 0.10f,  0.10f, -0.10f,  1.0f, 1.0f,
		 0.10f,  0.10f,  0.10f,  1.0f, 0.0f,
		 0.10f,  0.10f,  0.10f,  1.0f, 0.0f,
		-0.10f,  0.10f,  0.10f,  0.0f, 0.0f,
		-0.10f,  0.10f, -0.10f,  0.0f, 1.0f
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	return VAO;
}