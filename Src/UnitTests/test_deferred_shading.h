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
FCamera camera(glm::vec3(0.0f, 0.0f, 10.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void Draw_Scene(const FViewContext &InViewContext, FRenderPolicy &Policy);


class FDrawGeometryInfoShaderType : public FMeshShaderType
{
public:
	FDrawGeometryInfoShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FMeshShaderType(InVsFile, InPsFile)
	{
	}

	virtual void Prepare(const FViewContext &InView, const FMesh &InMesh)
	{
		FMeshShaderType::Prepare(InView, InMesh);
	}
};

class FDrawLightBoxShaderType : public FMeshShaderType
{
public:
	FDrawLightBoxShaderType(const std::string &InVsFile, const std::string &InPsFile)
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

// draw lighting scene
class FDeferredLightingShaderType : public FGlobalShaderType
{
public:
	FDeferredLightingShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FGlobalShaderType(InVsFile, InPsFile)
		, draw_mode(1)
	{
	}

	virtual void Prepare()
	{
		FGlobalShaderType::Prepare();

		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

		ProgramParams.push_back(new FShaderParameter_Integer1v("draw_mode", draw_mode));
		ProgramParams.push_back(new FShaderParameter_Integer1v("gPosition", 1));
		ProgramParams.push_back(new FShaderParameter_Integer1v("gNormal", 2));
		ProgramParams.push_back(new FShaderParameter_Integer1v("gAlbedoSpec", 3));
		ProgramParams.push_back(new FShaderParameter_Float3v("viewPos", glm::value_ptr(viewPos), 1));

		// Also send light relevant uniforms
		for (GLuint i = 0; i < lightPositions.size(); i++)
		{
			ProgramParams.push_back(new FShaderParameter_Float3v(("lights[" + std::to_string(i) + "].Position"), lightPositions[i].x, lightPositions[i].y, lightPositions[i].z));
			ProgramParams.push_back(new FShaderParameter_Float3v(("lights[" + std::to_string(i) + "].Color"), lightColors[i].x, lightColors[i].y, lightColors[i].z));
			
			// Update attenuation parameters and calculate radius
			const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const GLfloat linear = 0.7;
			const GLfloat quadratic = 1.8;
			// Then calculate radius of light volume/sphere
			const GLfloat lightThreshold = 5.0; // 5 / 256
			const GLfloat maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);
			GLfloat radius = (-linear + static_cast<float>(std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0 / lightThreshold) * maxBrightness)))) / (2 * quadratic);
			
			ProgramParams.push_back(new FShaderParameter_Float1v(("lights[" + std::to_string(i) + "].Linear"), linear));
			ProgramParams.push_back(new FShaderParameter_Float1v(("lights[" + std::to_string(i) + "].Quadratic"), quadratic));
			ProgramParams.push_back(new FShaderParameter_Float1v(("lights[" + std::to_string(i) + "].Radius"), radius));
		}

		GLDriver.SetTexture2D(1, gPositionTex);
		GLDriver.SetTexture2D(2, gNormalTex);
		GLDriver.SetTexture2D(3, gAlbedoSpecTex);
	}

public:
	int	draw_mode;
	FOpenGLTexture2DRef		gPositionTex;
	FOpenGLTexture2DRef		gNormalTex;
	FOpenGLTexture2DRef		gAlbedoSpecTex;

	glm::vec3			   viewPos;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
};


int CurrentDebugCategory = 1;


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


	// Load Shader
	TRefCountPtr<FDrawGeometryInfoShaderType> GeometryPass_Shader = new FDrawGeometryInfoShaderType("shaders/test_g_buffer.vs", "shaders/test_g_buffer.frag");
	TRefCountPtr<FDrawLightBoxShaderType> DrawLightBox_Shader = new FDrawLightBoxShaderType("shaders/test_deferred_light_box.vs", "shaders/test_deferred_light_box.frag");
	TRefCountPtr<FDeferredLightingShaderType> LightingPass_Shader = new FDeferredLightingShaderType("shaders/test_deferred_shading.vs", "shaders/test_deferred_shading.frag");

	FDrawFullQuadHelper QuadHelper;

	// Load Model
	Model = FModel::CreateModel("objects/MD5/Bob.md5mesh");//objects/nanosuit/nanosuit.obj");
	Model->InitRHI();

	Floor = FModel::CreatePlane("textures/wood.png");
	Floor->InitRHI();

	Cube = FModel::CreateCube("textures/wood.png");
	Cube->InitRHI();

	LightCube = FModel::CreateCube("textures/awesomeface.png");
	LightCube->InitRHI();

	/////
	// Setup G-Buffer
	// Create Frame Buffer
	FOpenGLFrameBufferRef GFrameBuffer = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef gPositionTex = GLDriver.CreateTexture2D(GL_RGB16F, screenWidth, screenHeight, GL_RGB, GL_FLOAT, nullptr);
	FOpenGLTexture2DRef gNormalTex = GLDriver.CreateTexture2D(GL_RGB16F, screenWidth, screenHeight, GL_RGB, GL_FLOAT, nullptr);
	FOpenGLTexture2DRef gAlbedoSpecTex = GLDriver.CreateTexture2D(GL_RGBA, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	FOpenGLRenderBufferRef DepthStencilBuffer = GLDriver.CreateRenderBuffer(GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	gPositionTex->SetFilterMode(GL_NEAREST, GL_NEAREST);
	gNormalTex->SetFilterMode(GL_NEAREST, GL_NEAREST);
	gAlbedoSpecTex->SetFilterMode(GL_NEAREST, GL_NEAREST);

	GFrameBuffer->SetColorAttachment(0, gPositionTex);
	GFrameBuffer->SetColorAttachment(1, gNormalTex);
	GFrameBuffer->SetColorAttachment(2, gAlbedoSpecTex);
	GFrameBuffer->SetDepthStencilAttachment(DepthStencilBuffer);
	GFrameBuffer->SetDrawBuffers(3, DrawBuffers);
	GFrameBuffer->CheckStatus();

	//生成场景物体位置信息
	std::vector<glm::vec3> objectPositions;
	objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));
	// - Colors
	const GLuint NR_LIGHTS = 32;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	srand(13);
	for (GLuint i = 0; i < NR_LIGHTS; i++)
	{
		// Calculate slightly random offsets
		GLfloat xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		GLfloat yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
		GLfloat zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
		// Also calculate random color
		GLfloat rColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
		GLfloat gColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
		GLfloat bColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
		lightColors.push_back(glm::vec3(rColor, gColor, bColor));
	}

	LightingPass_Shader->lightPositions = lightPositions;
	LightingPass_Shader->lightColors = lightColors;

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

		// PASS 1: Geometry Pass
		{
			GLDriver.SetFrameBuffer(GFrameBuffer);
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

			FRenderPolicy GeometryPolicy;
			GeometryPolicy.MeshShader = GeometryPass_Shader;

			for (size_t Index = 0; Index < objectPositions.size(); Index++)
			{
				viewContext.model =glm::translate(glm::mat4(), objectPositions[Index]);
				viewContext.model = glm::scale(viewContext.model, glm::vec3(0.25f));
				if (IsValidRef(Model))
				{
					Model->Draw(viewContext, GeometryPolicy);
				}
			} // end for Index

			// draw floor
			viewContext.model = glm::translate(glm::mat4(), glm::vec3(0.0f, -4.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
			if (IsValidRef(Floor))
			{
				Floor->Draw(viewContext, GeometryPolicy);
			}
		}

		// PASS 2: Lighting Pass
		{
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.f, 0.f, 0.f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
			LightingPass_Shader->viewPos = camera.Position;
			LightingPass_Shader->draw_mode = CurrentDebugCategory;
			LightingPass_Shader->gNormalTex = gNormalTex;
			LightingPass_Shader->gPositionTex = gPositionTex;
			LightingPass_Shader->gAlbedoSpecTex = gAlbedoSpecTex;

			QuadHelper.Draw((FDeferredLightingShaderType *)LightingPass_Shader, FOpenGLTexture2DRef());
		}

		// PASS 3: Draw Lights Cube
		if (1)
		{
			// 2.5. Copy content of geometry's depth buffer to default framebuffer's depth buffer
			GLDriver.BlitFramebuffer(GFrameBuffer, FOpenGLFrameBufferRef(), screenWidth, screenHeight, GL_DEPTH_BUFFER_BIT);

			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			// Create camera transformation
			glm::mat4 view = camera.GetViewMatrix();
			glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

			FViewContext viewContext;
			viewContext.viewport = glm::uvec4(0, 0, screenWidth, screenHeight);
			viewContext.view = view;
			viewContext.projection = projection;

			// Draw Light
			if (IsValidRef(LightCube))
			{
				FRenderPolicy DrawBoxPolicy;
				DrawBoxPolicy.MeshShader = DrawLightBox_Shader;

				for (GLuint i = 0; i < lightPositions.size(); i++)
				{
					glm::mat4 Pos = glm::translate(glm::mat4(), lightPositions[i]); // Translate it down a bit so it's at the center of the scene
					viewContext.model = glm::scale(Pos, glm::vec3(0.2f, 0.2f, 0.2f));

					DrawLightBox_Shader->LightColor = lightColors[i];
					LightCube->Draw(viewContext, DrawBoxPolicy);
				}
			}
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

	if (keys[GLFW_KEY_1])
		CurrentDebugCategory = 1;
	if (keys[GLFW_KEY_2])
		CurrentDebugCategory = 2;
	if (keys[GLFW_KEY_3])
		CurrentDebugCategory = 3;
	if (keys[GLFW_KEY_4])
		CurrentDebugCategory = 4;
	if (keys[GLFW_KEY_5])
		CurrentDebugCategory = 5;
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
