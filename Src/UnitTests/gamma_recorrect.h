#include <iostream>

#include "OpenGL/OpenGLDrv.h"
#include "Common/UtilityHelper.h"


// GLFW
#include <GLFW/glfw3.h>


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
	std::string  vShaderCode = FUtilityHelper::ReadTextFile("shaders/gamma_correct.vs");
	std::string  pShaderCode = FUtilityHelper::ReadTextFile("shaders/gamma_correct.frag");

	FOpenGLVertexShaderRef VertexShaderRef = GLDriver.CreateVertexShader(vShaderCode.c_str());
	FOpenGLPixelShaderRef PixelShaderRef = GLDriver.CreatePixelShader(pShaderCode.c_str());
	FOpenGLProgramRef ProgramRef = GLDriver.CreateProgram(VertexShaderRef, PixelShaderRef);

	VertexShaderRef->DumpDebugInfo();
	PixelShaderRef->DumpDebugInfo();
	ProgramRef->DumpDebugInfo();

	struct LVertex
	{
		GLfloat coord[2];
		GLfloat color;
	};

	#define kQuadsCount		100
	#define kQuadWidth		1.f
	#define kColorGrade		(1.f / kQuadsCount)
	
	LVertex vertices[kQuadsCount * 4];
	GLuint	indices[kQuadsCount * 6];

	for (GLuint k = 0; k < kQuadsCount; k++)
	{
		vertices[k * 4].coord[0] = kQuadWidth * k;
		vertices[k * 4].coord[1] = 0.f;
		vertices[k * 4].color = kColorGrade * k;


		vertices[k * 4 + 1].coord[0] = kQuadWidth *(k+1);
		vertices[k * 4 + 1].coord[1] = 0.f;
		vertices[k * 4 + 1].color = kColorGrade * k;

		vertices[k * 4 + 2].coord[0] = kQuadWidth *(k + 1);
		vertices[k * 4 + 2].coord[1] = kQuadWidth;
		vertices[k * 4 + 2].color = kColorGrade * k;

		vertices[k * 4 + 3].coord[0] = kQuadWidth * k;
		vertices[k * 4 + 3].coord[1] = kQuadWidth;
		vertices[k * 4 + 3].color = kColorGrade * k;
	
		indices[k * 6 + 0] = 0 + k*4;
		indices[k * 6 + 1] = 1 + k*4;
		indices[k * 6 + 2] = 3 + k*4;
		indices[k * 6 + 3] = 1 + k*4;
		indices[k * 6 + 4] = 2 + k*4;
		indices[k * 6 + 5] = 3 + k*4;
	} // end for k

#if 1
	FOpenGLVertexBufferRef VertexBufferRef = GLDriver.CreateVertexBuffer(sizeof(vertices), vertices);
	FOpenGLIndexBufferRef  IndexBufferRef = GLDriver.CreateIndexBuffer(sizeof(indices), indices, sizeof(GLuint));
	
	FVertexElementsList VertexElementList;
	VertexElementList.push_back(FVertexElement(0, 0, 0, sizeof(LVertex), VET_Float2));
	VertexElementList.push_back(FVertexElement(0, 1, STRUCT_VAR_OFFSET(LVertex, color), sizeof(LVertex), VET_Float1));
	FOpenGLVertexDeclarationRef VertexDeclRef = GLDriver.CreateVertexDeclaration(VertexElementList);

	GLDriver.SetStreamSource(0, VertexBufferRef);
	GLDriver.SetShaderProgram(ProgramRef);
	GLDriver.SetVertexDeclaration(VertexDeclRef);
#endif

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		// Set Shader Parameters
		FProgramParameters ProgramParams;

		glm::mat4 prjMat = glm::ortho(-10.f, float(kQuadsCount)+10.f, -10.f, +10.f);
		// Emit Draw Call 0
		{
			glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
			glm::mat4 mvp = prjMat * trans;
			ProgramParams.push_back(new FShaderParameter_Matrix4fv("mvp", mvp));
			ProgramParams.push_back(new FShaderParameter_UnInteger1v("bGamma", 0));
			GLDriver.SetShaderProgramParameters(&ProgramParams);
			GLDriver.DrawIndexedPrimitive(IndexBufferRef, GL_TRIANGLES, 0, sizeof(indices) / sizeof(indices[0]));
		}

		// Emit Draw Call 1
		if(1)
		{
			ProgramParams.clear();

			glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0.0f, 1.20f, 0.0f));
			glm::mat4 mvp = prjMat * trans;
			ProgramParams.push_back(new FShaderParameter_Matrix4fv("mvp", mvp));
			ProgramParams.push_back(new FShaderParameter_UnInteger1v("bGamma", 1));
			GLDriver.SetShaderProgramParameters(&ProgramParams);
			GLDriver.DrawIndexedPrimitive(IndexBufferRef, GL_TRIANGLES, 0, sizeof(indices) / sizeof(indices[0]));
		}


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