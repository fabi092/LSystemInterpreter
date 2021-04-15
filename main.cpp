#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <FreeImage.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader_s.h"
#include "camera.h"
#include "LSystem.h"

#include <iostream>
#include <stack>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
std::string askForFileName();
int askForPositiveInteger();

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// settings
	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 1024;
	std::string fileName = askForFileName();
	std::cout << "How many generations do you want to run?: " << std::flush;
	const unsigned int generations = askForPositiveInteger();
	std::cout << "By which degree do you want to rotate?: " << std::flush;
	const unsigned int rotationInDegree = askForPositiveInteger();
	const float rotation = rotationInDegree * 3.14159265359 / 180;
	glm::vec3 helpVector;

	//Initialize L-System
	LSystem lSystem = LSystem(fileName);
	for (uint32_t i = 0; i < generations; i++) {
		lSystem.simulateGeneration();
	}
	// variables for first run
	bool firstRun = true;
	float edgeRight = 0;
	float edgeLeft = 0;
	float edgeUp = 0;
	float edgeDown = 0;


	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader shader("shader.vert", "shader.frag");

	// shader configuration
	// --------------------
	shader.use();
	unsigned int VAO = 0;
	unsigned int VBO = 0;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// configure view/projection matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		glm::vec3 drawingDirection(0.0f, 0.1f, 0.0f);
		glm::vec3 currentLocation(0.0f, 0.0f, 0.0f);
		std::vector<glm::vec3> drawingPointsOfLSystem;
		std::stack<glm::vec3> stackOfPositions;
		std::stack<glm::vec3> stackOfDirections;

		for (char characterToCheck : lSystem.currentBuildInstructions) {
			if (characterToCheck == 'F') {

				drawingPointsOfLSystem.push_back(currentLocation);
				currentLocation += drawingDirection;
				drawingPointsOfLSystem.push_back(currentLocation);
				if (firstRun)
				{
					// Save the edges on first run to setup the camera correctly
					if (currentLocation.x > edgeRight)
						edgeRight = currentLocation.x;
					else if (currentLocation.x < edgeLeft)
						edgeLeft = currentLocation.x;

					if (currentLocation.y > edgeUp)
						edgeUp = currentLocation.y;
					else if (currentLocation.y < edgeDown)
						edgeDown = currentLocation.y;
				}
			}
			else if (characterToCheck == '[') {
				stackOfPositions.push(currentLocation);
				stackOfDirections.push(drawingDirection);
			}
			else if (characterToCheck == ']') {
				currentLocation = stackOfPositions.top();
				stackOfPositions.pop();

				drawingDirection = stackOfDirections.top();
				stackOfDirections.pop();
			}
			else if (characterToCheck == '+') {
				helpVector.x = cos(rotation) * drawingDirection.x - sin(rotation) * drawingDirection.y;
				helpVector.y = sin(rotation) * drawingDirection.x + cos(rotation) * drawingDirection.y;
				drawingDirection = helpVector;
			}
			else if (characterToCheck == '-') {
				helpVector.x = cos(-rotation) * drawingDirection.x - sin(-rotation) * drawingDirection.y;
				helpVector.y = sin(-rotation) * drawingDirection.x + cos(-rotation) * drawingDirection.y;
				drawingDirection = helpVector;
			}
		}
		// setup camera correctly on the first run
		if (firstRun) {
			camera.Position.x = edgeLeft + (edgeRight - edgeLeft) / 2;
			camera.Position.y = edgeDown + (edgeUp - edgeDown) / 2;
			float helper = std::_Max_value(camera.Position.x / windowWidth, camera.Position.y / windowHeight);
			camera.Position.z = helper * 2700;
			shader.setMat4("view", camera.GetViewMatrix());
		}
		// Draw the LSystem
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * drawingPointsOfLSystem.size(), &drawingPointsOfLSystem[0], GL_STATIC_DRAW);
		glDrawArrays(GL_LINES, 0, drawingPointsOfLSystem.size());
		glBindVertexArray(0);

		// Bitmap is saved on first run
		if (firstRun) {
			firstRun = false;
			glBindVertexArray(VAO);
			BYTE* pixels = new BYTE[3 * windowWidth * windowHeight];
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

			FIBITMAP* Image = FreeImage_ConvertFromRawBits(pixels, windowWidth, windowHeight, 3 * windowWidth, 24, 0x0000FF, 0xFF0000, 0x00FF00, false);
			FreeImage_Save(FIF_BMP, Image, "lastLSystem.bmp", 0);
			FreeImage_Unload(Image);
			delete[] pixels;
			glBindVertexArray(0);
		}
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

std::string askForFileName() {
	std::cout << "Please enter the name of the axiom file to use: " << std::flush;

	std::string  fileName;
	std::cin >> fileName;
	std::cout << std::endl;
	if (!std::cin.good()) {
		std::cin.clear();
		std::cout << "Please enter a vaild filename!" << std::endl << std::flush;

		return askForFileName();
	}
	else {
		return fileName;
	}
}

int askForPositiveInteger() {
	int nrOfGenerations;
	std::cin >> nrOfGenerations;
	std::cout << std::endl;
	if (!std::cin.good() || nrOfGenerations < 0) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << "Please enter a positive integer!" << std::endl;
		std::cout << std::endl<<std::flush;
		return askForPositiveInteger();
	}
	else {
		return nrOfGenerations;
	}
}