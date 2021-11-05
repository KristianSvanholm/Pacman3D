//Standard libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

//Glad & GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//GLM
#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/type_ptr.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"

using namespace std;

//Methods
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void readLevel(string path);
int initialize();

//Game logic
vector<vector<int>> level; //Level 2D array

//Screen
const float WIDTH = 280;
const float HEIGHT = 360;
const float ASPECT = WIDTH / HEIGHT;   // desired aspect ratio

GLFWwindow* window;

int main() {

	readLevel("../../../levels/level0");

	if (initialize() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	//initializeShader();

	//Gluint walls = createWallVao(); -> This should call createSquare();
	//Gluint pellets = createPelletVao(); -> This should call createSphere();

	//Create Pacman & Ghost vaos
	//GLuint pacman = CreatePacman(); -> This should only be a walking camera. figure out
	//GLuint Ghost = Load3DModel(path);

	// key catcher
	glfwSetKeyCallback(window, key_callback);

	while(!glfwWindowShouldClose(window)){

		//Compute Player Movement && Update pellets
		//Run ghost AI

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

void readLevel(string path) {
	int data;
	ifstream lvlFile(path);
	if (lvlFile)
	{
		string size;
		lvlFile >> size;
		int xMax = stoi(size.substr(0, 2));
		int yMax = stoi(size.substr(3));


		//Build array
		level = vector<vector<int>>(xMax);
		for (int i = 0; i < xMax; i++) {
			level[i] = vector<int>(yMax);
		}

		// Print current level
		cout << xMax << "*" << yMax << endl;
		for (int i = 0; i < yMax; i++) {
			for (int j = 0; j < xMax; j++) {
				lvlFile >> level[j][i];

				if (level[j][i] == 2) {
					//TODO instantiate Pacman at given coordinates
					//pac = new Pacman(j, i);
				}

				cout << level[j][i] << " ";
			}
			cout << endl;
		}

	}
	else {
		cout << "\n --Unable to read file " << path;
	}
	lvlFile.close();


	//Generate ghost position
	//RNG seeded by current time in seconds since January 1st, 1970
	srand(time(NULL));
	int randX, randY;
	do {
		randX = rand() % (level.size() - 1);
		randY = rand() % (level[1].size() - 1);
	} while (level[randX][randY] != 0); // checks for tunnel
	//TODO Instantiate new ghost(s?) at given position(s?)
	//ghost = new Ghost(randX, randY);
}