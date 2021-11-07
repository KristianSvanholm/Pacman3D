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
#include "glm/glm/gtc/type_ptr.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

//Learn OPGENL
#include "learnopengl/shader_m.h"
#include "learnopengl/filesystem.h"

using namespace std;

//Methods
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
vector<glm::vec3> readLevel(string path);
GLuint wallSegment();
int initialize();
void initializeShader();

//Game variables
vector<glm::vec3> level;

//Screen
const float WIDTH = 1000;
const float HEIGHT = 800;
const float ASPECT = WIDTH / HEIGHT;   // desired aspect ratio

GLFWwindow* window;

int main() {
	level = readLevel("../../../levels/level0");

	if (initialize() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}


	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("../../../shaders/7.1.camera.vs", "../../../shaders/7.1.camera.xs");

	initializeShader();

	GLuint walls = wallSegment();

	//Gluint pellets = createPelletVao(); -> This should call createSphere();

	//Create Pacman & Ghost vaos
	//GLuint pacman = CreatePacman(); -> This should only be a walking camera. figure out
	//GLuint Ghost = Load3DModel(path);

	// key catcher
	glfwSetKeyCallback(window, key_callback);

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use();
	ourShader.setInt("texture1", 0);
	ourShader.setInt("texture2", 1);

	// pass projection matrix to shader (as projection matrix rarely changes there's no need to do this per frame)
	// -----------------------------------------------------------------------------------------------------------
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	ourShader.setMat4("projection", projection);


	while(!glfwWindowShouldClose(window)){
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind textures on corresponding texture units
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, texture1);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, texture2);

		// activate shader
		ourShader.use();

		// camera/view transformation
		glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		float radius = 50.0f;
		float camX = sin(glfwGetTime()) * radius;
		float camZ = cos(glfwGetTime()) * radius;
		view = glm::lookAt(glm::vec3(camX, 1.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		ourShader.setMat4("view", view);

		glBindVertexArray(walls);
		for (unsigned int i = 0; i < level.size(); i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, level[i]);
			ourShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Compute Player Movement && Update pellets
		//Run ghost AI

		//TODO: Create draw function
		//Draw walls
		//Draw pellets
		//Draw ghost(s)

		glfwSwapBuffers(window);
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
	}

	//Termination of Stuff 
	//cleanVAO(); on all vaos
	glfwTerminate();
}

/// <summary>
/// Changes size of screen and everything in it
/// </summary>
/// <param name="window"> GLFW window </param>
/// <param name="width"> New width </param>
/// <param name="height"> New height </param>
void resizeCallback(GLFWwindow* window, int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int w = height * ASPECT;           // w is width adjusted for aspect ratio
	int left = (width - w) / 2;
	glViewport(left, 0, w, height);       // fix up the viewport to maintain aspect ratio
	glMatrixMode(GL_MODELVIEW);
}

/// <summary>
/// Called once every key is pressed and processes the command if valid
/// </summary>
/// <param name="window"> GLFW window </param>
/// <param name="key"> Key that was pressed </param>
/// <param name="scancode"> Not used, but required for method </param>
/// <param name="action"> Key press / release </param>
/// <param name="mods"> Not used, but required for method </param>
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {


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

vector<glm::vec3> readLevel(string path) {
	vector<glm::vec3> level;
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
				if (data == 2) {
					//TODO instantiate Pacman at given coordinates
					//pac = new Pacman(j, i);
				}

			}
			cout << endl;
		}

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
	return level;
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

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	// world space positions of our cubes
	glm::vec3 cubePositions[] = {
		glm::vec3(1,  0, 10),
		glm::vec3(2,  0, 9),
		glm::vec3(3,  0, 8),
		glm::vec3(4,  0, 7),
		glm::vec3(5,  0, 6),
		glm::vec3(6,  0, 5),
		glm::vec3(7,  0, 4),
		glm::vec3(8,  0, 3),
		glm::vec3(9,  0, 2),
		glm::vec3(10, 0, 1)
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

void initializeShader() {
	// load and create a texture 
	// -------------------------
	unsigned int texture1, texture2;
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(FileSystem::getPath("../../../resources/textures/container.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	// texture 2
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load(FileSystem::getPath("../../../resources/textures/awesomeface.png").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}