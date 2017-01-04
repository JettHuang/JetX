// \brief
//		Texture test
//

#include <iostream>

#include "Common/UtilityHelper.h"
#include "OpenGL/OpenGLDrv.h"

// GLFW
#include <GLFW/glfw3.h>

#include <SOIL.h>

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;


// The MAIN function, from here we start the application and run the game loop
int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;

	FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
	GLDriver.Initialize();

	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	// Define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLDriver.DeferredInitialize();


	// Load Shader
	std::string  vShaderCode = FUtilityHelper::ReadTextFile("shaders\\test_texture.vs");
	std::string  pShaderCode = FUtilityHelper::ReadTextFile("shaders\\test_texture.frag");

	FOpenGLVertexShaderRef VertexShaderRef = GLDriver.CreateVertexShader(vShaderCode.c_str());
	FOpenGLPixelShaderRef PixelShaderRef = GLDriver.CreatePixelShader(pShaderCode.c_str());
	FOpenGLProgramRef ProgramRef = GLDriver.CreateProgram(VertexShaderRef, PixelShaderRef);

	VertexShaderRef->DumpDebugInfo();
	PixelShaderRef->DumpDebugInfo();
	ProgramRef->DumpDebugInfo();

	// Set up vertex data (and buffer(s)) and attribute pointers
	struct FVertexInfo 
	{
		GLfloat	Position[3];
		GLfloat Color[3];
		GLfloat TexCoord0[2];
	};

	FVertexInfo vertices[] = 
	{
		// Positions          // Colors           // Texture Coords
		{{0.5f,  0.5f, 0.0f},   {1.0f, 0.0f, 0.0f},   {1.0f, 1.0f}}, // Top Right
		{{0.5f, -0.5f, 0.0f},   {0.0f, 1.0f, 0.0f},   {1.0f, 0.0f}}, // Bottom Right
		{{-0.5f, -0.5f, 0.0f},  {0.0f, 0.0f, 1.0f},   {0.0f, 0.0f}}, // Bottom Left
		{{-0.5f,  0.5f, 0.0f},  {1.0f, 1.0f, 0.0f},   {0.0f, 1.0f}}  // Top Left 
	};

	GLuint indices[] = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};

	FOpenGLVertexBufferRef VertexBufferRef = GLDriver.CreateVertexBuffer(sizeof(vertices), vertices);
	FOpenGLIndexBufferRef  IndexBufferRef = GLDriver.CreateIndexBuffer(sizeof(indices), indices, sizeof(GLuint));

	FVertexElementsList VertexElementList;
	VertexElementList.push_back(FVertexElement(0, 0,                   0, sizeof(FVertexInfo), VET_Float3));		// POSITION
	VertexElementList.push_back(FVertexElement(0, 1, sizeof(GLfloat) * 3, sizeof(FVertexInfo), VET_Float3));		// COLOR
	VertexElementList.push_back(FVertexElement(0, 2, sizeof(GLfloat) * 6, sizeof(FVertexInfo), VET_Float2));		// TEX-COORD

	FOpenGLVertexDeclarationRef VertexDeclRef = GLDriver.CreateVertexDeclaration(VertexElementList);

	GLDriver.SetStreamSource(0, VertexBufferRef);
	GLDriver.SetShaderProgram(ProgramRef);
	GLDriver.SetVertexDeclaration(VertexDeclRef);

#if 1
	FOpenGLTexture2DRef Texture2DRef0, Texture2DRef1;
	//Load Texture
	{
		int width, height;
		unsigned char* image = nullptr;
		
		image = SOIL_load_image("textures\\container.jpg", &width, &height, 0, SOIL_LOAD_RGB);
		Texture2DRef0 = GLDriver.CreateTexture2D(GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);

		image = SOIL_load_image("textures\\awesomeface.png", &width, &height, 0, SOIL_LOAD_RGB);
		Texture2DRef1 = GLDriver.CreateTexture2D(GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);

		GLDriver.SetTexture2D(0, Texture2DRef0);
		GLDriver.SetTexture2D(1, Texture2DRef1);
	}
#else
	{
		// Load and create a texture 
		GLuint texture1;
		GLuint texture2;
		// ====================
		// Texture 1
		// ====================
		glGenTextures(1, &texture1);
		glBindTexture(GL_TEXTURE_2D, texture1); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
												// Set our texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Load, create texture and generate mipmaps
		int width, height;

		unsigned char* image = SOIL_load_image("textures/container.jpg", &width, &height, 0, SOIL_LOAD_RGB);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
										 // ===================
										 // Texture 2
										 // ===================
		glGenTextures(1, &texture2);
		glBindTexture(GL_TEXTURE_2D, texture2);
		// Set our texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set texture filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Load, create texture and generate mipmaps
		image = SOIL_load_image("textures/awesomeface.png", &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
	}
#endif

	// Set Shader Parameters
	FProgramParameters ProgramParams;
	{
		ProgramParams.push_back(new FShaderParameter_Integer1v("ourTexture1", 0));
		ProgramParams.push_back(new FShaderParameter_Integer1v("ourTexture2", 1));
		GLDriver.SetShaderProgramParameters(&ProgramParams);
	}

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		GLDriver.DrawIndexedPrimitive(IndexBufferRef, GL_TRIANGLES, 0, 6);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
