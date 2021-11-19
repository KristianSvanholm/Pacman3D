//Standard libraries
#include <iostream>
#include <fstream>
#include <vector>
#include <set>

// Texture loader
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//LearnOPENGL.com header files
#include "learnopengl/shader_m.h"
#include "learnopengl/filesystem.h"

//Custom classes etc
#include "ghost.h"
#include "player.h"
#include "vaoHandler.h"

using namespace std;

//Methods
unsigned int initializeTexture(string path);
void DrawElements(vector<glm::vec3> elements, unsigned int texture, GLuint VAO, float scale, int vectorSize, Shader mainShader);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void readLevel(string path);
int initialize();

//World variables
vector<glm::vec3> level;
vector<glm::vec3> pellets;
vector<vector<int>> ghostLvl;
vector<Ghost*> ghosts;
Player* player;

//Game logic variables
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool win = false;
bool gameOver = false;

//Screen
const float WIDTH = 1000;
const float HEIGHT = 800;
GLFWwindow* window;

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
	glfwSetCursorPosCallback(window, mouseCallback);

	//Main game loop
	while(!glfwWindowShouldClose(window)){

		//##########################################################
		// GAME LOGIC PORTION
		//##########################################################
		
		//Deltatime calculation
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//pellet logic
		for (int i = 0; i < pellets.size(); i++) {
			//If pellets withing pickup range of player: remove it from vector
			if (glm::distance(pellets[i], player->getPosition()) < 0.5f) {
				pellets.erase(pellets.begin()+i);
			}
		}
		if (pellets.size() == 0) { //win condition
			win = true;
		}

		//ghost logic
		for (int i = 0; i < ghosts.size(); i++) {
			//If pellets withing pickup range of player: remove it from vector
			if (glm::distance(ghosts[i]->getPosition(), player->getPosition()) < 1.0f) {
				gameOver = true;
			}
		}
		for (int i = 0; i < ghosts.size(); i++) {
			ghosts[i]->updateGhost(deltaTime);
		}

		//userInput
		player->processInput(window, deltaTime);

		//##########################################################
		// DRAW PORTION
		//##########################################################
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader and apply player view
		ourShader.use();
		ourShader.setMat4("view", player->generateView());
		
		//Generate new position vector for ghosts for compatibility with DrawElements
		vector<glm::vec3> ghostPos;
		for (int i = 0; i < 4; i++) {ghostPos.push_back(ghosts[i]->getPosition());}
		//Draw walls, pellets and ghosts
		DrawElements(level, wallTexture, wallVAO, 1.0f , 36, ourShader);
		DrawElements(pellets, pelletTexture, pelletVAO, 0.3f, pelletSize, ourShader);
		DrawElements(ghostPos, ghostTexture, ghostVAO, 0.75f, ghostSize, ourShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//Termination of Stuff 
	CleanVAO(ghostVAO);
	CleanVAO(pelletVAO);
	CleanVAO(wallVAO);
	glfwTerminate();
}

/// <summary>
/// Draws a VAO at an array of positions, with texture and scale
/// </summary>
/// <param name="elements">Positions to draw VAOs at</param>
/// <param name="texture">Texture applied to VAOs</param>
/// <param name="VAO">VAO to draw</param>
/// <param name="scale">Scale to draw VAOs in</param>
/// <param name="vectorSize">Number of vertices in VAO</param>
/// <param name="mainShader">ShaderProgram</param>
void DrawElements(vector<glm::vec3> elements, unsigned int texture, GLuint VAO, float scale, int vectorSize, Shader shader) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(VAO);
	for (int i = 0; i < elements.size(); i++) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, elements[i]);
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, vectorSize);
	}
}

//Calls the same function in Player class as i couldnt apply the class function directly
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	player->mouseCallback(window, xpos, ypos);
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
					player = new Player(glm::vec3(i, 0, j), WIDTH/2, HEIGHT/2);
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