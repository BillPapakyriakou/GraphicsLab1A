//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1Α της OpenGL
//
//ΑΜ:   5186             Όνομα:  Βασιλείου Νικόλαος Μιχαήλ
//ΑΜ:   5324             Όνομα: Παπακυριακού Βασίλειος

//*********************************

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>


// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

//******************
// Η LoadShaders είναι black box για σας

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}



// 2D Array for storing the coords of the squares for the maze

int labyrinth[10][10] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{0, 0, 1, 1, 1, 1, 0, 1, 0, 1},
	{1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
	{1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
	{1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{1, 0, 1, 0, 1, 1, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


std::vector<float> vertices;   // for maze walls
std::vector<float> verticesA;  // for character A (player)


float C_x = -4.5f; // character x pos
float C_y = 2.5f;  // character y pos

GLuint vertexBufferPlayer;  // needed here instead of main because Player buffer
GLuint vertexBufferMaze;    // is constantly being updated on keyCallback method

void generateMazeVertices() {
	int rows = 10;
	int cols = 10;

	// Each square is of size 1x1, and we want to center the maze at (0, 0).

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {

			if (labyrinth[i][j] == 1) {

				// Calculate the corners of the square

				float topLeftX = j - cols / 2.0f;
				float topLeftY = (rows / 2.0f) - i;

				float topRightX = topLeftX + 1.0f;
				float topRightY = topLeftY;

				float bottomLeftX = topLeftX;
				float bottomLeftY = topLeftY - 1.0f;

				float bottomRightX = topRightX;
				float bottomRightY = bottomLeftY;

				// First triangle (top-left, bottom-left, top-right)

				vertices.push_back(topLeftX);
				vertices.push_back(topLeftY);
				vertices.push_back(0.0f); // z = 0 

				vertices.push_back(bottomLeftX);
				vertices.push_back(bottomLeftY);
				vertices.push_back(0.0f); // z = 0

				vertices.push_back(topRightX);
				vertices.push_back(topRightY);
				vertices.push_back(0.0f); // z = 0

				// Second triangle (bottom-left, bottom-right, top-right)

				vertices.push_back(bottomLeftX);
				vertices.push_back(bottomLeftY);
				vertices.push_back(0.0f); // z = 0

				vertices.push_back(bottomRightX);
				vertices.push_back(bottomRightY);
				vertices.push_back(0.0f); // z = 0

				vertices.push_back(topRightX);
				vertices.push_back(topRightY);
				vertices.push_back(0.0f); // z = 0
			}
		}
	}
}

	  
void DrawPlayer() {

	verticesA.clear();

	// First triangle (top-left, bottom-left, top-right)
	
	verticesA.push_back(C_x - 0.25f); 
	verticesA.push_back(C_y + 0.25f); 
	verticesA.push_back(0.0f);        // z = 0

	verticesA.push_back(C_x - 0.25f); 
	verticesA.push_back(C_y - 0.25f); 
	verticesA.push_back(0.0f);        // z = 0

	verticesA.push_back(C_x + 0.25f);
	verticesA.push_back(C_y + 0.25f);
	verticesA.push_back(0.0f);        // z = 0

	// Second triangle (bottom-left, bottom-right, top-right)

	verticesA.push_back(C_x - 0.25f); 
	verticesA.push_back(C_y - 0.25f); 
	verticesA.push_back(0.0f);        // z = 0

	verticesA.push_back(C_x + 0.25f); 
	verticesA.push_back(C_y - 0.25f); 
	verticesA.push_back(0.0f);        // z = 0

	verticesA.push_back(C_x + 0.25f); 
	verticesA.push_back(C_y + 0.25f); 
	verticesA.push_back(0.0f);        // z = 0

	
}

// returns true if move is valid
bool isValidMove(int newX, int newY) {


	int checkX = newX;
	int checkY = newY + 2; // because player starts at [0][2] 


	if (checkX < 0 || checkX >= 10 || checkY < 0 || checkY >= 10) {
		//std::cout << "Out of bounds: (" << checkX << ", " << checkY << ")" << std::endl;
		return false;
	}

	//std::cout << "(" << checkX << ", " << checkY << ")" << std::endl;

	return labyrinth[checkY][checkX] == 0;

}


// This function is called by GLFW whenever a key is pressed.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		
		int gridX = static_cast<int>(C_x + 4.5f); // Convert to grid position
		int gridY = static_cast<int>(2.5f - C_y); // Convert to grid position
		
		int newX = gridX;
		int newY = gridY;		

		// Handle movement keys
		switch (key) {
		case GLFW_KEY_L: newX += 1; break; // right
		case GLFW_KEY_J: newX -= 1; break; // left
		case GLFW_KEY_K: newY += 1; break; // down
		case GLFW_KEY_I: newY -= 1; break; // up
		}


		//std::cout << "Current Position: (" << C_x << ", " << C_y << ")" << std::endl;
		//std::cout << "Expected Position after move: (" << newX << ", " << newY << ")" << std::endl;

		
		// Check if the new move is valid before updating
		if (isValidMove(newX, newY)) {
			C_x = newX - 4.5f;
			C_y = 2.5f - newY;

			DrawPlayer();

			//std::cout << "valid move" << std::endl;

			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
			glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_DYNAMIC_DRAW);
		}

		
	}

}


int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(750, 750, "«Άσκηση 1Α - 2024", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Set key callback
	glfwSetKeyCallback(window, keyCallback);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//***********************************************
	// Οι shaders σας είναι οι 
    // ProjectVertexShader.vertexshader
    // ProjectFragmentShader.fragmentshader

	GLuint programID = LoadShaders("ProjectVertexShader.vertexshader", "ProjectFragmentShader.fragmentshader");
	
    ///////////////////////////////////////////////////////////////////////////////////////	
	/**Το παρακάτω το αγνοείτε - είναι τοποθέτηση κάμερας ***/
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	glm::mat4 Projection = glm::perspective(glm::radians(30.0f), 4.0f / 4.0f, 0.1f, 100.0f);
	
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0, 30), // Camera  in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // 
	);

	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model; 
    ///////////////////////////////////////////////////////////////////////////////////////
	//**************************************************
	/// Για βοήθεια το πρόγραμμα αναπαριστά ενα τυχαίο τρίγωνο - εσείς θα πρέπει να βάλετε κορυφές κατάλληλες 
	//  για το δικό σας τρίγωνο.
	//Στην άσκηση αυτή δουλεύετε στις 2 διαστάσεις x,y οπότε η z συνιστώσα θα ειναι πάντα 0.0f
	
	static const GLfloat shape_1_buffer[] = {
		 3.0f, 3.0f, 0.0f,
		 0.0f, 3.0f, 0.0f,
		 0.0f,  0.0f, 0.0f	
	};

	// draw maze 
	generateMazeVertices();

	// draw Player
    DrawPlayer();

	// for testing purposes only
	//std::cout << "Total vertices: " << vertices.size() << std::endl;

	// Create buffers
	//GLuint vertexBufferMaze, vertexBufferPlayer;

	// Generate and bind buffer for the maze
	glGenBuffers(1, &vertexBufferMaze);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMaze);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	// Generate and bind buffer for the player
	glGenBuffers(1, &vertexBufferPlayer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
	glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_STATIC_DRAW);

	
	
	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);  /// Αυτό αφορά την κάμερα  - το αγνοείτε

		// Draw maze walls
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMaze);
		glVertexAttribPointer(
			0,                  // attribute 0, must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
		glDisableVertexAttribArray(0);


		// Draw player
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
		glVertexAttribPointer(
			0,                 // attribute 0, must match the layout in the shader.
			3,				   // size
			GL_FLOAT,          // type
			GL_FALSE,          // normalized?
			0,                 // stride
			(void*)0           // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, verticesA.size() / 3);
		glDisableVertexAttribArray(0);

			
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} 
	//while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
	while (glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS &&  glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexBufferMaze);
	glDeleteBuffers(1, &vertexBufferPlayer);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

