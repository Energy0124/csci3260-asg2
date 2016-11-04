/*********************************************************
FILE : submit.cpp (csci3260 2016-2017 Assignment 2)
*********************************************************/
/*********************************************************
Student Information
Student ID: 1155062557
Student Name: Ling Leong
*********************************************************/

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include "Dependencies\freeglut\freeglut_ext.h"
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;
//using namespace glm;
using glm::vec3;
using glm::vec2;
using glm::mat4;
using glm::radians;

GLint programID;
// Could define the Vao&Vbo and interaction parameter here
// Window dimensions
GLuint WIDTH = 800, HEIGHT = 600;
glm::vec3 cubePositions[10];

GLint xAxis = 0, yAxis = 0, zAxis = 0;
GLfloat deltaX = 0.005f, deltaY = 0.005f, deltaZ = 0.005f;
GLfloat scaleFactor = 1, deltaScale = 1.005f;

GLint rotation = 0;
GLfloat deltaAngle = 0.01f;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

GLfloat deltaCameraTime = 0.0f;	// Time between current frame and last frame
GLfloat lastCameraFrame = 0.0f;  	// Time of last frame


GLfloat mouseControl_y_sensitivity = 2.0f;
GLfloat camera_sensitivity = 0.1f;
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;

GLfloat yaw = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch = 0.0f;

GLfloat fov = radians(45.0f);

bool keys[1024] = { false };
bool special_keys[1024] = { false };

GLuint VBO_Tetrahedron, VAO_Tetrahedron, EBO_Tetrahedron;
GLuint VBO_Ground, VAO_Ground, EBO_Ground;
GLuint VBO_Jeep, VAO_Jeep, UV_Jeep, Normal_Jeep, drawSize_Jeep, texture_Jeep, specular_Jeep;
GLuint VBO_Plane, VAO_Plane, UV_Plane, Normal_Plane, drawSize_Plane, texture_Plane, specular_Plane;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
};

GLfloat sLightIntensityFactor = 1, sLightIntensityDeltaFactor = 1.005f;
GLfloat dLightIntensityFactor = 1, dLightIntensityDeltaFactor = 1.005f;

vec3 spotDLightColor = vec3(0.8f, 0.3f, 0.8f);
vec3 spotSLightColor = vec3(0.8f, 0.3f, 0.8f);

vec3 pointDLightColor = vec3(0.8f, 0.8f, 0.8f);
vec3 pointSLightColor = vec3(1.0f, 1.0f, 1.0f);

vec3 directionDLightColor = vec3(0.4f, 0.4f, 0.4f);
vec3 directionSLightColor = vec3(0.5f, 0.5f, 0.5f);


GLboolean wireframeMode = false;

GLboolean freeCameraMode = false;

GLboolean autoRotate = true;

GLulong autoRotateCount = 0;

//time for calculating FPS
int lastTime = 0;
int deltaTime = 0;
int deltaFrame = 0;

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShader.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShader.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void keyboard(unsigned char key, int x, int y)
{
	if (key >= 0 && key < 1024)
	{
		keys[key] = true;
	}


}
void keyboardUp(unsigned char key, int x, int y)
{
	if (key >= 0 && key < 1024)
	{
		keys[key] = false;
	}
}



void handleKeys()
{

	if (special_keys[GLUT_KEY_UP])
	{
		zAxis -= 1;
	}
	if (special_keys[GLUT_KEY_DOWN])
	{
		zAxis += 1;
	}
	if (special_keys[GLUT_KEY_LEFT])
	{
		xAxis -= 1;
	}
	if (special_keys[GLUT_KEY_RIGHT])
	{
		xAxis += 1;
	}

	/*	if (keys['a'])
		{
			xAxis -= 1;
		}
		if (keys['d'])
		{
			xAxis += 1;
		}
		if (keys['w'])
		{
			zAxis -= 1;
		}
		if (keys['s'])
		{
			zAxis += 1;
		}*/
	if (keys['9'])
	{
		yAxis -= 1;
	}
	if (keys['0'])
	{
		yAxis += 1;
	}
	if (keys['='])
	{
		scaleFactor *= deltaScale;
	}
	if (keys['-'])
	{
		scaleFactor /= deltaScale;
	}
	if (keys['t'])
	{
		rotation += 1;
	}
	if (keys['g'])
	{
		rotation -= 1;
	}
	if (keys['m'])
	{
		wireframeMode = !wireframeMode;
		keys['m'] = false;
	}
	//reset everything
	if (keys['/'])
	{
		xAxis = 0, yAxis = 0, zAxis = 0;
		deltaX = 0.005f, deltaY = 0.005f, deltaZ = 0.005f;
		scaleFactor = 1, deltaScale = 1.005f;
		rotation = 0;
		deltaAngle = 0.01f;

		// Camera
		cameraPos = glm::vec3(0.0f, 2.0f, 10.0f);
		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

		camera_sensitivity = 0.1f;
		lastX = WIDTH / 2.0f;
		lastY = HEIGHT / 2.0f;
		firstMouse = true;

		yaw = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
		pitch = 0.0f;

		fov = radians(45.0f);

		sLightIntensityFactor = 1, sLightIntensityDeltaFactor = 1.005f;
		dLightIntensityFactor = 1, dLightIntensityDeltaFactor = 1.005f;

		spotDLightColor = vec3(0.8f, 0.3f, 0.8f);
		spotSLightColor = vec3(0.8f, 0.3f, 0.8f);

		pointDLightColor = vec3(0.8f, 0.8f, 0.8f);
		pointSLightColor = vec3(1.0f, 1.0f, 1.0f);

		directionDLightColor = vec3(0.4f, 0.4f, 0.4f);
		directionSLightColor = vec3(0.5f, 0.5f, 0.5f);


		wireframeMode = false;
		freeCameraMode = false;

	}

	/*	if (keys['c'])
		{
			//		static int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
			//		static int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
			//		lastX = centerX;
			//		lastY = centerY;
			yaw = -90.0f;
			pitch = 0.0f;
			cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
			firstMouse = true;

		}*/
		//camera sensitivity
	if (keys[','])
	{

		camera_sensitivity -= 0.001f;
		if (camera_sensitivity < 0.05) camera_sensitivity = 0.05;
	}
	if (keys['.'])
	{
		camera_sensitivity += 0.001f;
		if (camera_sensitivity > 0.5) camera_sensitivity = 0.5;
	}

	if (keys[27])
	{
		glutLeaveMainLoop();
	}

	//camera and control mode
	if (keys['c'])
	{
		freeCameraMode = !freeCameraMode;
		keys['c'] = false;
	}

	if (keys['s'])
	{
		autoRotate = !autoRotate;
		keys['s'] = false;
	}

	if (keys['q'])
	{

		spotDLightColor *= dLightIntensityDeltaFactor;
		pointDLightColor *= dLightIntensityDeltaFactor;
		directionDLightColor *= dLightIntensityDeltaFactor;
		//keys['q'] = false;
	}
	if (keys['w'])
	{
		spotDLightColor /= dLightIntensityDeltaFactor;
		pointDLightColor /= dLightIntensityDeltaFactor;
		directionDLightColor /= dLightIntensityDeltaFactor;

		//keys['w'] = false;
	}
	if (keys['z'])
	{
		spotSLightColor *= sLightIntensityDeltaFactor;
		pointSLightColor *= sLightIntensityDeltaFactor;
		directionSLightColor *= sLightIntensityDeltaFactor;
		//keys['z'] = false;
	}
	if (keys['x'])
	{
		spotSLightColor /= sLightIntensityDeltaFactor;
		pointSLightColor /= sLightIntensityDeltaFactor;
		directionSLightColor /= sLightIntensityDeltaFactor;
		//keys['x'] = false;
	}


}

void handleSpecialKeypress(int key, int x, int y) {
	if (key >= 0 && key < 1024)
	{
		special_keys[key] = true;
	}


}

void handleSpecialKeyReleased(int key, int x, int y) {
	if (key >= 0 && key < 1024)
	{
		special_keys[key] = false;
	}
}

void ActiveMouse(int x, int y)
{
	if (!freeCameraMode)
	{
		return;
	}


	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;
	lastX = (GLfloat)x;
	lastY = (GLfloat)y;


	xoffset *= camera_sensitivity;
	yoffset *= camera_sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void PassiveMouse(int x, int y)
{
	if (firstMouse)
	{
		lastX = (GLfloat)x;
		lastY = (GLfloat)y;
		firstMouse = false;
	}


	//TODO: Use Mouse to do interactive events and animation
	if (!freeCameraMode)
	{
		GLfloat xoffset = x - lastX;
		GLfloat yoffset = lastY - y;
		//xoffset *= camera_sensitivity;
		yoffset *= mouseControl_y_sensitivity;
		yAxis -= yoffset;

	}
	lastX = (GLfloat)x;
	lastY = (GLfloat)y;
}

void handleMouseWheel(int wheel, int direction, int x, int y)
{
	if (fov >= radians(1.0f) && fov <= radians(60.0f))
		fov -= direction*0.05f;
	//cout << direction<<" "<<fov <<endl;
	if (fov <= radians(1.0f))
		fov = radians(1.0f);
	if (fov >= radians(60.0f))
		fov = radians(60.0f);
}

void updateRatio()
{
	WIDTH = glutGet(GLUT_WINDOW_WIDTH) / 2;
	HEIGHT = glutGet(GLUT_WINDOW_HEIGHT) / 2;

};

void calculateFPS()
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	if (lastTime == 0) lastTime = currentTime;
	else if (deltaTime >= 1000)
	{
		printf("FPS: %d\n", deltaFrame);

		deltaFrame = 1;

		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
	}
	else
	{
		deltaFrame++;
		deltaTime += currentTime - lastTime;
		lastTime = currentTime;
	}

};

void updateCamera()
{
	//camX = sin((GLfloat)glutGet(GLUT_ELAPSED_TIME) * 0.0001f) * radius;
	//camZ = cos((GLfloat)glutGet(GLUT_ELAPSED_TIME) * 0.0001f) * radius;

	GLfloat currentFrame = (GLfloat)glutGet(GLUT_ELAPSED_TIME);
	deltaCameraTime = currentFrame - lastCameraFrame;
	lastCameraFrame = currentFrame;
	// Camera controls
	GLfloat cameraSpeed = 0.01f*deltaCameraTime;

	/*	if (special_keys[GLUT_KEY_UP])
			cameraPos += cameraSpeed * cameraFront;
		if (special_keys[GLUT_KEY_DOWN])
			cameraPos -= cameraSpeed * cameraFront;
		if (special_keys[GLUT_KEY_LEFT])
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (special_keys[GLUT_KEY_RIGHT])
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;*/

	if (keys['i'])
		cameraPos += cameraSpeed * cameraFront;
	if (keys['k'])
		cameraPos -= cameraSpeed * cameraFront;
	if (keys['j'])
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keys['l'])
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

};

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width*height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);


	GLuint textureID;
	//TODO: Create one OpenGL texture and set the texture parameter 

	glGenTextures(1, &textureID);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	// OpenGL has now copied the data. Free our own version
	delete[] data;

	//unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}


void bindJeepObj()
{
	// Read our .obj file
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;
	bool res = loadOBJ("jeep.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &VAO_Jeep);
	glBindVertexArray(VAO_Jeep);

	glGenBuffers(1, &VBO_Jeep);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Jeep);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &UV_Jeep);
	glBindBuffer(GL_ARRAY_BUFFER, UV_Jeep);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &Normal_Jeep);
	glBindBuffer(GL_ARRAY_BUFFER, Normal_Jeep);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	drawSize_Jeep = vertices.size();
	// 1rst attribute buffer : vertices

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Jeep);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, UV_Jeep);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, Normal_Jeep);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO*/
};

void bindPlaneObj()
{
	// Read our .obj file
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;
	bool res = loadOBJ("plane.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &VAO_Plane);
	glBindVertexArray(VAO_Plane);

	glGenBuffers(1, &VBO_Plane);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Plane);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &UV_Plane);
	glBindBuffer(GL_ARRAY_BUFFER, UV_Plane);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &Normal_Plane);
	glBindBuffer(GL_ARRAY_BUFFER, Normal_Plane);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	drawSize_Plane = vertices.size();
	// 1rst attribute buffer : vertices

	glBindBuffer(GL_ARRAY_BUFFER, VBO_Plane);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, UV_Plane);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, Normal_Plane);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		0, // stride
		(void*)0 // array buffer offset
	);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO*/
};
void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture

	//bindTetrahedron();
	//bindGround();
	bindJeepObj();
	bindPlaneObj();

	//lighting
	glUniform1i(glGetUniformLocation(programID, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(programID, "material.specular"), 1);

}


void drawJeep()
{
	// Create transformations
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	//model = glm::rotate(model, (GLfloat)glutGet(GLUT_ELAPSED_TIME) * 0.001f, glm::vec3(0.5f, 1.0f, 0.0f));
	//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	//view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	// Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
	projection = glm::perspective(fov, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);



	model = glm::translate(model, glm::vec3(deltaX * xAxis, deltaY * yAxis, deltaZ * zAxis));
	if (autoRotate)
	{
		autoRotateCount++;
		model = glm::rotate(model, deltaAngle*(rotation+autoRotateCount/5) , glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else {
		model = glm::rotate(model, deltaAngle*(rotation + autoRotateCount/5), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	model = glm::scale(model, scaleFactor*glm::vec3(1.0f));


	// Get their uniform location
	GLint modelLoc = glGetUniformLocation(programID, "model");
	GLint viewLoc = glGetUniformLocation(programID, "view");
	GLint projLoc = glGetUniformLocation(programID, "projection");

	// Pass them to the shaders

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	GLint viewPosLoc = glGetUniformLocation(programID, "viewPos");
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	// Set material properties
	glUniform1f(glGetUniformLocation(programID, "material.shininess"), 32.0f);


	// == ==========================
	// Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
	// the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
	// by defining light types as classes and set their values in there, or by using a more efficient uniform approach
	// by using 'Uniform buffer objects', but that is something we discuss in the 'Advanced GLSL' tutorial.
	// == ==========================
	// Directional light
	glUniform3f(glGetUniformLocation(programID, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
	glUniform3f(glGetUniformLocation(programID, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "dirLight.diffuse"), directionDLightColor.x, directionDLightColor.y, directionDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "dirLight.specular"), directionSLightColor.x, directionSLightColor.y, directionSLightColor.z);
	// Point light 1
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].quadratic"), 0.032);
	// Point light 2
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].quadratic"), 0.032);
	// Point light 3
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].quadratic"), 0.032);
	// Point light 4
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].quadratic"), 0.032);
	// SpotLight
	glUniform3f(glGetUniformLocation(programID, "spotLight.position"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.direction"), cameraFront.x, cameraFront.y, cameraFront.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(programID, "spotLight.diffuse"), spotDLightColor.x, spotDLightColor.y, spotDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.specular"), spotSLightColor.x, spotSLightColor.y, spotSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "spotLight.quadratic"), 0.032);
	glUniform1f(glGetUniformLocation(programID, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(programID, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

	// Bind diffuse map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_Jeep);
	// Bind specular map
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specular_Jeep);


	/*	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Jeep);
		glUniform1i(TextureID, 0);*/


		// Draw container
	glBindVertexArray(VAO_Jeep);
	glDrawArrays(GL_TRIANGLES, 0, drawSize_Jeep);
	glBindVertexArray(0);
}

void drawPlane()
{
	// Create transformations
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;


	//model = glm::rotate(model, (GLfloat)glutGet(GLUT_ELAPSED_TIME) * 0.001f, glm::vec3(0.5f, 1.0f, 0.0f));
	//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	//view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	// Note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
	projection = glm::perspective(fov, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);



	model = glm::translate(model, glm::vec3(deltaX * xAxis, deltaY * yAxis - 1, deltaZ * zAxis));
	model = glm::rotate(model, deltaAngle*rotation, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, scaleFactor*glm::vec3(1.0f));


	// Get their uniform location
	GLint modelLoc = glGetUniformLocation(programID, "model");
	GLint viewLoc = glGetUniformLocation(programID, "view");
	GLint projLoc = glGetUniformLocation(programID, "projection");
	// Pass them to the shaders

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	GLint viewPosLoc = glGetUniformLocation(programID, "viewPos");
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
	// Set material properties
	glUniform1f(glGetUniformLocation(programID, "material.shininess"), 32.0f);

	// == ==========================
	// Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
	// the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
	// by defining light types as classes and set their values in there, or by using a more efficient uniform approach
	// by using 'Uniform buffer objects', but that is something we discuss in the 'Advanced GLSL' tutorial.
	// == ==========================
	// Directional light
	glUniform3f(glGetUniformLocation(programID, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
	glUniform3f(glGetUniformLocation(programID, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "dirLight.diffuse"), directionDLightColor.x, directionDLightColor.y, directionDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "dirLight.specular"), directionSLightColor.x, directionSLightColor.y, directionSLightColor.z);
	// Point light 1
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].quadratic"), 0.032);
	// Point light 2
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].quadratic"), 0.032);
	// Point light 3
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].quadratic"), 0.032);
	// Point light 4
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].ambient"), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].diffuse"), pointDLightColor.x, pointDLightColor.y, pointDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].specular"), pointSLightColor.x, pointSLightColor.y, pointSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].quadratic"), 0.032);
	// SpotLight
	glUniform3f(glGetUniformLocation(programID, "spotLight.position"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.direction"), cameraFront.x, cameraFront.y, cameraFront.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(programID, "spotLight.diffuse"), spotDLightColor.x, spotDLightColor.y, spotDLightColor.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.specular"), spotSLightColor.x, spotSLightColor.y, spotSLightColor.z);
	glUniform1f(glGetUniformLocation(programID, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.linear"), 0.09);
	glUniform1f(glGetUniformLocation(programID, "spotLight.quadratic"), 0.032);
	glUniform1f(glGetUniformLocation(programID, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(programID, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

	// Bind diffuse map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_Plane);
	// Bind specular map
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specular_Plane);



	/*	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Plane);
		glUniform1i(TextureID, 0);*/


		// Draw container
	glBindVertexArray(VAO_Plane);
	glDrawArrays(GL_TRIANGLES, 0, drawSize_Plane);
	glBindVertexArray(0);
}

void paintGL(void)
{//render your objects and control the transformation here
	glClearColor(0.4f, 0.6f, 1.0f, 1.0f); //background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (wireframeMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	updateRatio();

	updateCamera();

	handleKeys();

	//drawTetrahedron();

//	drawGround();

	drawJeep();
	drawPlane();

	glFlush();
	glutPostRedisplay();

	calculateFPS();
}

void loadTexture()
{
	texture_Jeep = loadBMP_custom("jeep_texture.bmp");
	specular_Jeep = loadBMP_custom("jeep_texture_specular.bmp");
	//texture_Plane = loadBMP_custom("plane_texture.bmp");
	texture_Plane = loadBMP_custom("container2.bmp");
	specular_Plane = loadBMP_custom("container2_specular.bmp");
}

void initializedGL(void)
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 4);
	installShaders();
	sendDataToOpenGL();
	loadTexture();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_MULTISAMPLE);
	//TODO:
	//Create a window with title specified
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Chiaki Nanami -- 1155062557");
	//glutSetCursor(GLUT_CURSOR_NONE);

	initializedGL();
	glViewport(0, 0, WIDTH, HEIGHT);
	glutDisplayFunc(paintGL);
	glutPassiveMotionFunc(PassiveMouse);
	glutMotionFunc(ActiveMouse);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(handleSpecialKeypress);
	glutSpecialUpFunc(handleSpecialKeyReleased);
	glutMouseWheelFunc(handleMouseWheel);

	glutMainLoop();

	return 0;
}


