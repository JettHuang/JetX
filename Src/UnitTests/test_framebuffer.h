// Std. Includes
#include <string>

#include "Common/UtilityHelper.h"
#include "OpenGL/OpenGLDrv.h"
#include "Scene/Camera.h"
#include "Scene/Model.h"
#include "Scene/Render.h"
#include "Scene/ShaderType.h"


// GLFW
#include <GLFW/glfw3.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Camera
FCamera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// The MAIN function, from here we start our application and run our Game loop
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

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	GLDriver.DeferredInitialize();

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);
	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);


	// Load Shader
#if 0
	std::string  vShaderCode = FUtilityHelper::ReadTextFile("shaders/test_model.vs");
	std::string  pShaderCode = FUtilityHelper::ReadTextFile("shaders/test_model.frag");

	FOpenGLVertexShaderRef VertexShaderRef = GLDriver.CreateVertexShader(vShaderCode.c_str());
	FOpenGLPixelShaderRef PixelShaderRef = GLDriver.CreatePixelShader(pShaderCode.c_str());
	FOpenGLProgramRef ProgramRef = GLDriver.CreateProgram(VertexShaderRef, PixelShaderRef);

	VertexShaderRef->DumpDebugInfo();
	PixelShaderRef->DumpDebugInfo();
	ProgramRef->DumpDebugInfo();
#else
	TRefCountPtr<FMeshShaderType> MeshShader = new FMeshShaderType("shaders/test_model.vs", "shaders/test_model.frag");
#endif

	// Load Postprocess Shader
	std::string vPostShaderCode = FUtilityHelper::ReadTextFile("shaders/fullscreen_quad.vs");
	std::string pPostShaderCode = FUtilityHelper::ReadTextFile("shaders/fullscreen_quad.frag");

	FOpenGLVertexShaderRef PostVertexShaderRef = GLDriver.CreateVertexShader(vPostShaderCode.c_str());
	FOpenGLPixelShaderRef  PostPixelShaderRef = GLDriver.CreatePixelShader(pPostShaderCode.c_str());
	FOpenGLProgramRef PostProgramRef = GLDriver.CreateProgram(PostVertexShaderRef, PostPixelShaderRef);
	PostVertexShaderRef->DumpDebugInfo();
	PostPixelShaderRef->DumpDebugInfo();
	PostProgramRef->DumpDebugInfo();

	// Load Model
	FModelRef Model = FModel::CreateModel("objects/nanosuit/nanosuit.obj");
	Model->InitRHI();

	// Create Frame Buffer;
	FOpenGLFrameBufferRef FrameBuffer = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef SceneTexture = GLDriver.CreateTexture2D(GL_RGB, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	FOpenGLRenderBufferRef DepthStencilBuffer = GLDriver.CreateRenderBuffer(GL_DEPTH24_STENCIL8, screenWidth, screenHeight);

	FrameBuffer->SetColorAttachment(0, SceneTexture);
	FrameBuffer->SetDepthStencilAttachment(DepthStencilBuffer);
	FrameBuffer->CheckStatus();

	// quad
	GLfloat quadVerts[] = {
		0.f, 0.f, 0.f, 0.f,  // v0, t0
		1.f, 0.f, 1.f, 0.f,  // v1, t1
		1.f, 1.f, 1.f, 1.f,  // v2, t2
		0.f, 1.f, 0.f, 1.f   // v3, t3
	};
	GLuint quadIndices[] = {
		0, 1, 2,   // triangle 0
		0, 2, 3    // triangle 1
	};

	FVertexElementsList quadElements;
	quadElements.push_back(FVertexElement(0, 0, 0, 4 * sizeof(GLfloat), VET_Float2));
	quadElements.push_back(FVertexElement(0, 1, 2 * sizeof(GLfloat), 4 * sizeof(GLfloat), VET_Float2));

	FOpenGLVertexBufferRef  quadVertBuffer = GLDriver.CreateVertexBuffer(sizeof(quadVerts), quadVerts);
	FOpenGLIndexBufferRef   quadIndexBuffer = GLDriver.CreateIndexBuffer(sizeof(quadIndices), quadIndices, sizeof(quadIndices[0]));
	FOpenGLVertexDeclarationRef quadVertDecl = GLDriver.CreateVertexDeclaration(quadElements);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Do_Movement();

		// PASS 1:
		GLDriver.SetFrameBuffer(FrameBuffer);

		SceneTexture->SetFilterMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

		// Clear the colorbuffer
		GLDriver.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create camera transformation
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

		FViewContext viewContext;
		viewContext.viewport = glm::uvec4(0, 0, screenWidth, screenHeight);
		viewContext.view = view;
		viewContext.projection = projection;

		viewContext.model = glm::translate(viewContext.model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		viewContext.model = glm::scale(viewContext.model, glm::vec3(0.2f, 0.2f, 0.2f));

		if (IsValidRef(Model))
		{
			FRenderPolicy RenderPolicy;
			RenderPolicy.MeshShader = MeshShader;

			Model->Draw(viewContext, RenderPolicy);
			GLDriver.CheckError(__FILE__, __LINE__);
		}

		// PASS 2:
		if(1)
		{
			// post-processing
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());

			// Clear the colorbuffer
			GLDriver.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			SceneTexture->SetFilterMode(GL_LINEAR, GL_LINEAR);

			// SHADER parameters
			FProgramParameters Parameters;
			glm::mat4 MVP = glm::ortho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);
			Parameters.push_back(new FShaderParameter_Matrix4fv("mvp", MVP));
			Parameters.push_back(new FShaderParameter_Integer1v("sceneTex", 0));
			
			GLDriver.SetShaderProgram(PostProgramRef);
			GLDriver.SetShaderProgramParameters(&Parameters);

			GLDriver.SetTexture2D(0, SceneTexture);
			GLDriver.SetStreamSource(0, quadVertBuffer);
			GLDriver.SetVertexDeclaration(quadVertDecl);
			GLDriver.DrawIndexedPrimitive(quadIndexBuffer, GL_TRIANGLES, 0, 6);
		}
		// Swap the buffers
		glfwSwapBuffers(window);
	}

	Model->ReleaseRHI();
	// Properly de-allocate all resources once they've outlived their purpose
	glfwTerminate();
	return 0;
}

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//cout << key << endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
