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
GLuint depthWidth = 1024, depthHeight = 1024;

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

void Draw_Scene(const FViewContext &InViewContext, FRenderPolicy &Policy);

class FShadowShaderType : public FMeshShaderType
{
public:
	FShadowShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FMeshShaderType(InVsFile, InPsFile)
	{
	}

};

class FLightingShaderType : public FMeshShaderType
{
public:
	FLightingShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FMeshShaderType(InVsFile, InPsFile)
	{
	}

	virtual void Prepare(const FViewContext &InView, const FMesh &InMesh)
	{
		FMeshShaderType::Prepare(InView, InMesh);

		ProgramParams.push_back(new FShaderParameter_Float3v("lightPos", glm::value_ptr(LightPos), 1));
		ProgramParams.push_back(new FShaderParameter_Float3v("viewPos", glm::value_ptr(ViewPos), 1));
		ProgramParams.push_back(new FShaderParameter_Matrix4fv("ShadowVP", ShadowVP));
		ProgramParams.push_back(new FShaderParameter_Integer1v("shadowMapTex", 3));

		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
		GLDriver.SetTexture2D(3, ShadowMapTex);
	}

public:
	glm::vec3		LightPos;
	glm::vec3		ViewPos;
	glm::mat4		ShadowVP;

	FOpenGLTexture2DRef	ShadowMapTex;
};

class FShowDepthShaderType : public FGlobalShaderType
{
public:
	FShowDepthShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FGlobalShaderType(InVsFile, InPsFile)
	{
	}

	virtual void Prepare()
	{
		FGlobalShaderType::Prepare();

		ProgramParams.push_back(new FShaderParameter_Float1v("near_plane", 0.1f));
		ProgramParams.push_back(new FShaderParameter_Float1v("far_plane", 100.f));
	}
};

FModelRef Model, Floor, Cube, LightCube;
bool bShowDepthMap = false;

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

	glm::vec3 LightPos(0.f, 5.f, 0.f);

	// Load Shader
	TRefCountPtr<FLightingShaderType> LightingShader = new FLightingShaderType("shaders/test_shadow_mapping_lighting.vs", "shaders/test_shadow_mapping_lighting.frag");
	LightingShader->LightPos = LightPos;

	TRefCountPtr<FShadowShaderType> ShadowShader = new FShadowShaderType("shaders/test_shadow_mapping_depth.vs", "shaders/test_shadow_mapping_depth.frag");
	TRefCountPtr<FGlobalShaderType> DisplayDepthShader = new FShowDepthShaderType("shaders/test_shadow_mapping_showdepth.vs", "shaders/test_shadow_mapping_showdepth.frag");
	TRefCountPtr<FMeshShaderType> MeshShader = new FMeshShaderType("shaders/test_model.vs", "shaders/test_model.frag");

	FDrawFullQuadHelper QuadHelper;

	// Load Model
	Model = FModel::CreateModel("objects/nanosuit/nanosuit.obj");
	Model->InitRHI();

	Floor = FModel::CreatePlane("textures/wood.png");
	Floor->InitRHI();

	Cube = FModel::CreateCube("textures/wood.png");
	Cube->InitRHI();

	LightCube = FModel::CreateCube("textures/awesomeface.png");
	LightCube->InitRHI();

	// Create Frame Buffer;
	FOpenGLFrameBufferRef FrameBuffer = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef DepthTexture = GLDriver.CreateTexture2D(GL_DEPTH_COMPONENT, depthWidth, depthHeight, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	DepthTexture->SetWrapMode(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
	DepthTexture->SetFilterMode(GL_LINEAR, GL_LINEAR);
	DepthTexture->SetBorderColor(1.f, 1.f, 1.f, 1.f);


	FrameBuffer->SetDepthAttachment(DepthTexture);
	FrameBuffer->SetDrawBuffer(GL_NONE);
	FrameBuffer->SetReadBuffer(GL_NONE);

	FTexture2DRef TestTexture = FTexture2D::CreateTexture("textures/wood.png");
	TestTexture->InitRHI();

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

		glm::mat4 LightView = glm::lookAt(LightPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
		glm::mat4 LightProjection = glm::perspective(60.f, (float)depthWidth / (float)depthHeight, 0.2f, 100.f);

// pass 0: draw depth
		if(1)
		{
			GLDriver.SetFrameBuffer(FrameBuffer);
		
			glViewport(0, 0, depthWidth, depthHeight);
			// Clear the buffer
			GLDriver.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			FViewContext viewContext;
			viewContext.viewport = glm::uvec4(0, 0, depthWidth, depthHeight);
			viewContext.view = LightView;
			viewContext.projection = LightProjection;

			FRenderPolicy ShadowPolicy;
			ShadowPolicy.MeshShader = ShadowShader;

			Draw_Scene(viewContext, ShadowPolicy);
		}

// pass1: draw scene
		if(1)
		{
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Create camera transformation
			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

			FViewContext viewContext;
			viewContext.viewport = glm::uvec4(0, 0, screenWidth, screenHeight);
			viewContext.view = view;
			viewContext.projection = projection;

			// update view position
			LightingShader->ViewPos = camera.Position;

			// shadow vp
			glm::mat4 ShadowVP = LightProjection * LightView;
			LightingShader->ShadowVP = ShadowVP;
			LightingShader->ShadowMapTex = DepthTexture;

			FRenderPolicy LightingPolicy;
			LightingPolicy.MeshShader = LightingShader;

			Draw_Scene(viewContext, LightingPolicy);

			// Debug: Draw Light
			if (IsValidRef(LightCube))
			{
				glm::mat4 LightWorld = glm::translate(glm::mat4(), LightPos); // Translate it down a bit so it's at the center of the scene
				LightWorld = glm::scale(LightWorld, glm::vec3(0.2f, 0.2f, 0.2f));
				viewContext.model = LightWorld;

				FRenderPolicy NormalPolicy;
				NormalPolicy.MeshShader = MeshShader;

				LightCube->Draw(viewContext, NormalPolicy);
			}
		}

// debug: show depth map
		if (bShowDepthMap)
		{
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			QuadHelper.Draw(DisplayDepthShader, DepthTexture);
		}

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	Model->ReleaseRHI();
	// Properly de-allocate all resources once they've outlived their purpose
	glfwTerminate();
	return 0;
}

void Draw_Scene(const FViewContext &InViewContext, FRenderPolicy &Policy)
{
	glm::mat4 temp;
	FViewContext viewContext = InViewContext;

	temp = glm::translate(glm::mat4(), glm::vec3(-3.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	viewContext.model = glm::scale(temp, glm::vec3(0.2f, 0.2f, 0.2f));

	if (IsValidRef(Model))
	{
		Model->Draw(viewContext, Policy);
	}

	temp = glm::translate(glm::mat4(), glm::vec3(0.0f, -4.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	viewContext.model = temp;
	if (IsValidRef(Floor))
	{
		Floor->Draw(viewContext, Policy);
	}

	// DRAW CUBES
	temp = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
	viewContext.model = temp;
	if (IsValidRef(Cube))
	{
		Cube->Draw(viewContext, Policy);
	}
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

	if (keys[GLFW_KEY_B])
	{
		bShowDepthMap = !bShowDepthMap;
	}
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
