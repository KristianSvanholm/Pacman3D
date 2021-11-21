#ifndef vaoHandler_header
#define vaoHandler_header

//Tiny object loader
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
#include <GLFW/glfw3.h>

using namespace std;

GLuint loadModel(const string path, const string file, int& size);
void cleanVAO(GLuint& vao);
GLuint wallSegment();

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
GLuint loadModel(const std::string path, const std::string file, int& size)
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
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (path + file).c_str(), (path).c_str());

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

/// <summary>
/// Deletes data within VAO
/// </summary>
/// <param name="vao">VAO to be cleaned</param>
void cleanVAO(GLuint& vao)
{
	GLint nAttr = 0;
	std::set<GLuint> vbos;

	GLint eboId;
	glGetVertexArrayiv(vao, GL_ELEMENT_ARRAY_BUFFER_BINDING, &eboId);
	glDeleteBuffers(1, (GLuint*)&eboId);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttr);
	glBindVertexArray(vao);

	for (int iAttr = 0; iAttr < nAttr; ++iAttr)
	{
		GLint vboId = 0;
		glGetVertexAttribiv(iAttr, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vboId);
		if (vboId > 0)
		{
			vbos.insert(vboId);
		}

		glDisableVertexAttribArray(iAttr);
	}

	for (auto vbo : vbos)
	{
		glDeleteBuffers(1, &vbo);
	}

	glDeleteVertexArrays(1, &vao);
}

/// <summary>
/// VAO for wall segments of the map
/// </summary>
/// <returns>New VAO</returns>
GLuint wallSegment() {
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

		 //Removed top and bottom faces as they will never be seen anyways
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	return VAO;
}

#endif
