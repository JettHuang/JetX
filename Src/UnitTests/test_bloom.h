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

void Draw_Scene(const FViewContext &InViewContext, FRenderPolicy &Policy);


class FBloomPassOneShaderType : public FMeshShaderType
{
public:
	FBloomPassOneShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FMeshShaderType(InVsFile, InPsFile)
	{
	}

	virtual void Prepare(const FViewContext &InView, const FMesh &InMesh)
	{
		FMeshShaderType::Prepare(InView, InMesh);

		ProgramParams.push_back(new FShaderParameter_Float3v("light.Position", glm::value_ptr(LightPos), 1));
		ProgramParams.push_back(new FShaderParameter_Float3v("light.Color", glm::value_ptr(LightColor), 1));
		ProgramParams.push_back(new FShaderParameter_Float3v("viewPos", glm::value_ptr(ViewPos), 1));
	}

public:
	glm::vec3		LightPos;
	glm::vec3		LightColor;
	glm::vec3		ViewPos;
};

class FBloomPassOne_DrawLightShaderType : public FMeshShaderType
{
public:
	FBloomPassOne_DrawLightShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FMeshShaderType(InVsFile, InPsFile)
	{
	}

	virtual void Prepare(const FViewContext &InView, const FMesh &InMesh)
	{
		FMeshShaderType::Prepare(InView, InMesh);

		ProgramParams.push_back(new FShaderParameter_Float3v("lightColor", glm::value_ptr(LightColor), 1));
	}

public:
	glm::vec3		LightColor;
};

class FBloomPassBlurType : public FGlobalShaderType
{
public:
	FBloomPassBlurType(const std::string &InVsFile, const std::string &InPsFile)
		: FGlobalShaderType(InVsFile, InPsFile)
		, bHorizontal(true)
	{
	}

	virtual void Prepare()
	{
		FGlobalShaderType::Prepare();

		ProgramParams.push_back(new FShaderParameter_Integer1v("bHorizontal", bHorizontal));
	}

public:
	int	bHorizontal;
};

class FBloomPassFinalType : public FGlobalShaderType
{
public:
	FBloomPassFinalType(const std::string &InVsFile, const std::string &InPsFile)
		: FGlobalShaderType(InVsFile, InPsFile)
	{
	}

	virtual void Prepare()
	{
		FGlobalShaderType::Prepare();

		ProgramParams.push_back(new FShaderParameter_Integer1v("bloomBlur", 1));

		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();
		GLDriver.SetTexture2D(1, BloomBlurTex);
	}

public:
	FOpenGLTexture2DRef		BloomBlurTex;
};

enum DebugCategory
{
	kBloomResult = 0,
	kBrighnessTexture,
	kBlurHorizontal,
	kBlurResult,
	kMaxCategory
};

int CurrentDebugCategory = kBloomResult;


FModelRef Model, Floor, Cube, LightCube;


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

	glm::vec3 LightPos(2.f, 2.f, 2.f);
	glm::vec3 LightColor(2.0f, 2.0f, 2.0f);

	// Load Shader
	TRefCountPtr<FBloomPassOneShaderType> BloomPass1_Shader = new FBloomPassOneShaderType("shaders/bloom_pass1.vs", "shaders/bloom_pass1.frag");
	BloomPass1_Shader->LightPos = LightPos;
	BloomPass1_Shader->LightColor = LightColor;

	TRefCountPtr<FBloomPassOne_DrawLightShaderType> BloomPass1_DrawLight_Shader = new FBloomPassOne_DrawLightShaderType("shaders/bloom_pass1.vs", "shaders/bloom_pass1_drawlight.frag");
	BloomPass1_DrawLight_Shader->LightColor = LightColor;

	TRefCountPtr<FBloomPassBlurType> ScreenQuadShaderBlur = new FBloomPassBlurType("shaders/bloom_blur.vs", "shaders/bloom_blur.frag");
	TRefCountPtr<FBloomPassFinalType> ScreenQuadShaderFinal = new FBloomPassFinalType("shaders/bloom_final.vs", "shaders/bloom_final.frag");
	TRefCountPtr<FGlobalShaderType> ScreenQuadShader = new FGlobalShaderType("shaders/bloom_debug_quad.vs", "shaders/bloom_debug_quad.frag");

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

	// Create Frame Buffer
	FOpenGLFrameBufferRef FrameBuffer = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef SceneTexture0 = GLDriver.CreateTexture2D(GL_RGB, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	FOpenGLTexture2DRef SceneTexture1 = GLDriver.CreateTexture2D(GL_RGB, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	FOpenGLRenderBufferRef DepthStencilBuffer = GLDriver.CreateRenderBuffer(GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	SceneTexture0->SetFilterMode(GL_LINEAR, GL_LINEAR);
	SceneTexture1->SetFilterMode(GL_LINEAR, GL_LINEAR);
	FrameBuffer->SetColorAttachment(0, SceneTexture0);
	FrameBuffer->SetColorAttachment(1, SceneTexture1);
	FrameBuffer->SetDepthStencilAttachment(DepthStencilBuffer);
	FrameBuffer->SetDrawBuffers(2, DrawBuffers);
	FrameBuffer->CheckStatus();

	// Frame Buffer For Blur
	FOpenGLFrameBufferRef FrameBufferBlurHori = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef SceneTextureBlurHori = GLDriver.CreateTexture2D(GL_RGB, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	SceneTextureBlurHori->SetFilterMode(GL_LINEAR, GL_LINEAR);
	FrameBufferBlurHori->SetColorAttachment(0, SceneTextureBlurHori);
	FrameBufferBlurHori->CheckStatus();

	FOpenGLFrameBufferRef FrameBufferBlurVert = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef SceneTextureBlurVert = GLDriver.CreateTexture2D(GL_RGB, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	SceneTextureBlurVert->SetFilterMode(GL_LINEAR, GL_LINEAR);
	FrameBufferBlurVert->SetColorAttachment(0, SceneTextureBlurVert);
	FrameBufferBlurVert->CheckStatus();


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

		// pass1: draw scene and extract the bright part 
		if (1)
		{
			GLDriver.SetFrameBuffer(FrameBuffer);
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.f, 0.f, 0.f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Create camera transformation
			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

			FViewContext viewContext;
			viewContext.viewport = glm::uvec4(0, 0, screenWidth, screenHeight);
			viewContext.view = view;
			viewContext.projection = projection;

			// update view position
			BloomPass1_Shader->ViewPos = camera.Position;

			FRenderPolicy LightingPolicy;
			LightingPolicy.MeshShader = BloomPass1_Shader;

			Draw_Scene(viewContext, LightingPolicy);

			// Draw Light
			if (IsValidRef(LightCube))
			{
				glm::mat4 LightWorld = glm::translate(glm::mat4(), LightPos); // Translate it down a bit so it's at the center of the scene
				LightWorld = glm::scale(LightWorld, glm::vec3(0.2f, 0.2f, 0.2f));
				viewContext.model = LightWorld;

				FRenderPolicy NormalPolicy;
				NormalPolicy.MeshShader = BloomPass1_DrawLight_Shader;

				LightCube->Draw(viewContext, NormalPolicy);
			}
		}

		// pass2: horizontal blur
		if (1)
		{
			GLDriver.SetFrameBuffer(FrameBufferBlurHori);
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.f, 0.f, 0.f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT);
			ScreenQuadShaderBlur->bHorizontal = true;

			QuadHelper.Draw((FBloomPassBlurType *)ScreenQuadShaderBlur, SceneTexture1);
		}
		// pass3: vertical blur
		if (1)
		{
			GLDriver.SetFrameBuffer(FrameBufferBlurVert);
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.f, 0.f, 0.f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT);
			ScreenQuadShaderBlur->bHorizontal = false;

			QuadHelper.Draw((FBloomPassBlurType *)ScreenQuadShaderBlur, SceneTextureBlurHori);
		}

		// pass4: Now, We get a gassian blur image, and let us blend it with origion scene texture 
		if (1)
		{
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.f, 0.f, 0.f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ScreenQuadShaderFinal->BloomBlurTex = SceneTextureBlurVert;

			QuadHelper.Draw(ScreenQuadShaderFinal.DeRef(), SceneTexture0);
		}

		// debug
		if (CurrentDebugCategory)
		{
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.f, 0.f, 0.f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			FOpenGLTexture2DRef QuadTex;
			switch (CurrentDebugCategory)
			{
			case kBrighnessTexture:
				QuadTex = SceneTexture1; break;
			case kBlurHorizontal:
				QuadTex = SceneTextureBlurHori; break;
			case kBlurResult:
				QuadTex = SceneTextureBlurVert; break;
			default:
				break;
			}

			QuadHelper.Draw(ScreenQuadShader, QuadTex);
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

	if (keys[GLFW_KEY_SPACE])
	{
		CurrentDebugCategory = (CurrentDebugCategory + 1) % kMaxCategory;
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
