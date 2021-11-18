//Standard libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <cmath>

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

//Tiny object loader
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

//Custom
#include "ghost.h";

using namespace std;

//Methods
void DrawElements(vector<glm::vec3> elements, unsigned int texture, GLuint VAO, float scale, int vectorSize, Shader mainShader);
GLuint LoadModel(const std::string path, const std::string file, int& size);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int initializeTexture(string path);
void processInput(GLFWwindow* window);
void movePlayer(glm::vec3 input);
bool collides(glm::vec3 pos);
void readLevel(string path);
GLuint wallSegment();
int initialize();
//------

//Game variables
vector<glm::vec3> level;
vector<glm::vec3> pellets;
vector<vector<int>> ghostLvl;
vector<Ghost*> ghosts;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool win = false;
bool gameOver = false;
//-------------

//Screen
const float WIDTH = 1000;
const float HEIGHT = 800;
GLFWwindow* window;
//-----

//Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch;
float lastX = WIDTH / 2, lastY = HEIGHT / 2;
bool firstMouse = true;
// --------------

int main() {

	readLevel("../../../levels/level0");

	//initalizes all the libraries used
	if (initialize() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader program
	Shader ourShader("../../../shaders/7.1.camera.vs", "../../../shaders/7.1.camera.frag");

	// load and create a texture from path
	unsigned int wallTexture = initializeTexture("../../../../resources/textures/wall.jpg");
	unsigned int pelletTexture = initializeTexture("../../../../resources/textures/yellow.jpg");
	unsigned int ghostTexture = initializeTexture("../../../../resources/textures/tex.jpg");

	//Loads in and creates VAO for all models
	GLuint wallVAO = wallSegment();
	int pelletSize = 0, ghostSize = 0;
	GLuint pelletVAO = LoadModel("../../../resources/model/pellets/", "globe-sphere.obj", pelletSize);
	GLuint ghostVAO = LoadModel("../../../resources/model/ghost/","pacman-ghosts.obj", ghostSize);

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

		for (int i = 0; i < ghosts.size(); i++) {
			//If pellets withing pickup range of player: remove it from vector
			if (glm::distance(ghosts[i]->getPosition(), cameraPos) < 1.0f) {
				gameOver = true;
			}
		}

		//userInput
		processInput(window);

		//Ghost
		for (int i = 0; i < ghosts.size(); i++) {
			ghosts[i]->updateGhost(0, deltaTime);
		}

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
		DrawElements(level, wallTexture, wallVAO, 1.0f , 36, ourShader);
		
		//Draw pellets && bind texture
		DrawElements(pellets, pelletTexture, pelletVAO, 0.3f, pelletSize, ourShader);

		vector<glm::vec3> ghostPos;
		for (int i = 0; i < 4; i++) {
			ghostPos.push_back(ghosts[i]->getPosition());
		}
		DrawElements(ghostPos, ghostTexture, ghostVAO, 0.75f, ghostSize, ourShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Termination of Stuff 
	//TODO::cleanVAO(); on all vaos
	glfwTerminate();
}

void DrawElements(vector<glm::vec3> elements, unsigned int texture, GLuint VAO, float scale, int vectorSize, Shader mainShader) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(VAO);
	for (int i = 0; i < elements.size(); i++) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, elements[i]);
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		mainShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, vectorSize);
	}
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

/// <summary>
/// Takes in all legal input from player and handles it.
/// </summary>
/// <param name="window">Window to get input data from</param>
void processInput(GLFWwindow* window)
{
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
/// Checks and applies input to player if possible
/// </summary>
/// <param name="input">New input from player controller</param>
void movePlayer(glm::vec3 input) {
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
bool collides(glm::vec3 pos) {
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

		ghostLvl = vector<vector<int>>(xMax);
		for (int i = 0; i < xMax; i++) {
			ghostLvl[i] = vector<int>(yMax);
		}

		// Print current level
		cout << xMax << "*" << yMax << endl;
		for (int i = 0; i < yMax; i++) {
			for (int j = 0; j < xMax; j++) {
				int data;
				lvlFile >> data;
				if (data == 1) {
					level.push_back(glm::vec3(i, 0, j));
					ghostLvl[j][i] = 1;
				}
				else if (data == 0) {
					pellets.push_back(glm::vec3(i, -0.25, j));
					ghostLvl[j][i] = 0;
				}
				else if (data == 2) {
					ghostLvl[j][i] = 0;
					cameraPos.x = i;
					cameraPos.z = j;
				}

			}
			cout << endl;
		}

		//Generate ghost position
		//RNG seeded by current time in seconds since January 1st, 1970
		srand(time(NULL));
		for (int i = 0; i < 4; i++) {
			//Pellets contain all walkable space in map so a random pick from pellets will give a valid location
			glm::vec3 pos = pellets[rand() % pellets.size()];
			ghosts.push_back(new Ghost(ghostLvl, pos.z, pos.x));
			
			srand(rand()); //reseed rng
		}
	}
	else {
		cout << "\n --Unable to read file " << path;
	}
	lvlFile.close();
}

/// <summary>
/// VAO for wall segments of the map
/// </summary>
/// <returns>New VAO</returns>
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

//Data structure used in the following function
struct Vertex
{
	glm::vec3 location;
	glm::vec3 normals;
	glm::vec2 texCoords;
};

/// <summary>
/// Loads 3D model from path
/// </summary>
/// <param name="path">Path to look</param>
/// <param name="file">Which obj file to get</param>
/// <param name="size">Size callback variable</param>
/// <returns>Newly generated VAO for model</returns>
GLuint LoadModel(const std::string path, const std::string file, int& size)
{
	//We create a vector of Vertex structs. OpenGL can understand these, and so will accept them as input.
	vector<Vertex> vertices;

	//Some variables that we are going to use to store data from tinyObj
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials; //This one goes unused for now, seeing as we don't need materials for this model.

	//Some variables incase there is something wrong with our obj file
	string warn;
	string err;

	//We use tinobj to load our models. Feel free to find other .obj files and see if you can load them.
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (path+file).c_str(), (path).c_str());

	if (!warn.empty()) {
		cout << warn << std::endl;
	}

	if (!err.empty()) {
		cerr << err << std::endl;
	}

	//For each shape defined in the obj file
	for (auto shape : shapes)
	{
		//We find each mesh
		for (auto meshIndex : shape.mesh.indices)
		{
			//And store the data for each vertice, including normals
			glm::vec3 vertice = {
				attrib.vertices[meshIndex.vertex_index * 3],
				attrib.vertices[(meshIndex.vertex_index * 3) + 1],
				attrib.vertices[(meshIndex.vertex_index * 3) + 2]
			};
			glm::vec3 normal = {
				attrib.normals[meshIndex.normal_index * 3],
				attrib.normals[(meshIndex.normal_index * 3) + 1],
				attrib.normals[(meshIndex.normal_index * 3) + 2]
			};
			glm::vec2 textureCoordinate = {                         //These go unnused, but if you want textures, you will need them.
				attrib.texcoords[meshIndex.texcoord_index * 2],
				attrib.texcoords[(meshIndex.texcoord_index * 2) + 1]
			};

			vertices.push_back({ vertice, normal, textureCoordinate }); //We add our new vertice struct to our vector

		}
	}

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//As you can see, OpenGL will accept a vector of structs as a valid input here
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, nullptr);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	//This will be needed later to specify how much we need to draw. Look at the main loop to find this variable again.
	size = vertices.size();

	return VAO;
}
