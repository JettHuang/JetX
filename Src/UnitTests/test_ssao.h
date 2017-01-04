// Std. Includes
#include <string>
#include <random>

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

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}


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

// calculate ssao factors
class FCalculateAmbientOcclusionShaderType : public FGlobalShaderType
{
public:
	FCalculateAmbientOcclusionShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FGlobalShaderType(InVsFile, InPsFile)
	{

	}

	virtual void Prepare()
	{
		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

		FGlobalShaderType::Prepare();
		
		ProgramParams.push_back(new FShaderParameter_Integer1v("gPositionDepth", 1));
		ProgramParams.push_back(new FShaderParameter_Integer1v("gNormal", 2));
		ProgramParams.push_back(new FShaderParameter_Integer1v("texNoise", 3));
		ProgramParams.push_back(new FShaderParameter_Float3v("samples[0]", (GLfloat *)kernelSamples.data(), kernelSamples.size()));
		ProgramParams.push_back(new FShaderParameter_Matrix4fv("projection", projection));

		GLDriver.SetTexture2D(1, gPositionDepthTex);
		GLDriver.SetTexture2D(2, gNormalTex);
		GLDriver.SetTexture2D(3, texNoiseTex);
	}

public:
	FOpenGLTexture2DRef		gPositionDepthTex;
	FOpenGLTexture2DRef		gNormalTex;
	FOpenGLTexture2DRef		texNoiseTex;
	std::vector<glm::vec3>	kernelSamples;
	glm::mat4				projection;
};

// blur the ssao factors texture
class FAmbientOcclusionBlurShaderType : public FGlobalShaderType
{
public:
	FAmbientOcclusionBlurShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FGlobalShaderType(InVsFile, InPsFile)
	{

	}

	virtual void Prepare()
	{
		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

		FGlobalShaderType::Prepare();

		ProgramParams.push_back(new FShaderParameter_Integer1v("ssaoInput", 1));

		GLDriver.SetTexture2D(1, ssaoInputTex);
	}

public:
	FOpenGLTexture2DRef		ssaoInputTex;
};

// draw lighting scene
class FSSAOLightingShaderType : public FGlobalShaderType
{
public:
	FSSAOLightingShaderType(const std::string &InVsFile, const std::string &InPsFile)
		: FGlobalShaderType(InVsFile, InPsFile)
		, draw_mode(1)
	{
	}

	virtual void Prepare()
	{
		FGlobalShaderType::Prepare();

		FOpenGLDrv &GLDriver = FOpenGLDrv::SharedInstance();

		ProgramParams.push_back(new FShaderParameter_Integer1v("draw_mode", draw_mode));
		ProgramParams.push_back(new FShaderParameter_Integer1v("gPositionDepth", 1));
		ProgramParams.push_back(new FShaderParameter_Integer1v("gNormal", 2));
		ProgramParams.push_back(new FShaderParameter_Integer1v("gAlbedoSpec", 3));
		ProgramParams.push_back(new FShaderParameter_Integer1v("ssao", 4));

		// Also send light relevant uniforms
		ProgramParams.push_back(new FShaderParameter_Float3v(("light.Position"), lightPosition.x, lightPosition.y, lightPosition.z));
		ProgramParams.push_back(new FShaderParameter_Float3v(("light.Color"), lightColor.x, lightColor.y, lightColor.z));
			
		// Update attenuation parameters and calculate radius
		const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		const GLfloat linear = 0.2;
		const GLfloat quadratic = 0.3;

		ProgramParams.push_back(new FShaderParameter_Float1v(("light.Linear"), linear));
		ProgramParams.push_back(new FShaderParameter_Float1v(("light.Quadratic"), quadratic));



		GLDriver.SetTexture2D(1, gPositionDepthTex);
		GLDriver.SetTexture2D(2, gNormalTex);
		GLDriver.SetTexture2D(3, gAlbedoSpecTex);
		GLDriver.SetTexture2D(4, ssaoFactorsTex);
	}

public:
	int	draw_mode;
	FOpenGLTexture2DRef		gPositionDepthTex;
	FOpenGLTexture2DRef		gNormalTex;
	FOpenGLTexture2DRef		gAlbedoSpecTex;
	FOpenGLTexture2DRef		ssaoFactorsTex;

	glm::vec3		lightPosition;
	glm::vec3		lightColor;
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
	TRefCountPtr<FDrawGeometryInfoShaderType> GeometryPass_Shader = new FDrawGeometryInfoShaderType("shaders/test_ssao_gbuffer.vs", "shaders/test_ssao_gbuffer.frag");
	TRefCountPtr<FCalculateAmbientOcclusionShaderType> AmbientOcclusion_Shader = new FCalculateAmbientOcclusionShaderType("shaders/test_ssao.vs", "shaders/test_ssao.frag");
	TRefCountPtr<FAmbientOcclusionBlurShaderType> AmbientOcclusionBlur_Shader = new FAmbientOcclusionBlurShaderType("shaders/test_ssao.vs", "shaders/test_ssao_blur.frag");
	TRefCountPtr<FSSAOLightingShaderType> SSAOLighting_Shader = new FSSAOLightingShaderType("shaders/test_ssao.vs", "shaders/test_ssao_lighting.frag");
	TRefCountPtr<FDrawLightBoxShaderType> DrawLightBox_Shader = new FDrawLightBoxShaderType("shaders/test_deferred_light_box.vs", "shaders/test_deferred_light_box.frag");
	TRefCountPtr<FGlobalShaderType> DebugNormalQuad_Shader = new FGlobalShaderType("shaders/fullscreen_quad.vs", "shaders/fullscreen_quad.frag");
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

	// Lights
	glm::vec3 lightPos = glm::vec3(2.0, 3.0, 4.0);
	glm::vec3 lightColor = glm::vec3(0.8, 0.8, 0.8);


	/////
	// Setup G-Buffer
	// Create Frame Buffer
	FOpenGLFrameBufferRef GFrameBuffer = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef gPositionDepthTex = GLDriver.CreateTexture2D(GL_RGBA16F, screenWidth, screenHeight, GL_RGB, GL_FLOAT, nullptr);
	FOpenGLTexture2DRef gNormalTex = GLDriver.CreateTexture2D(GL_RGB, screenWidth, screenHeight, GL_RGB, GL_FLOAT, nullptr);
	FOpenGLTexture2DRef gAlbedoSpecTex = GLDriver.CreateTexture2D(GL_RGBA, screenWidth, screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	FOpenGLRenderBufferRef DepthStencilBuffer = GLDriver.CreateRenderBuffer(GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	gPositionDepthTex->SetFilterMode(GL_NEAREST, GL_NEAREST);
	gNormalTex->SetFilterMode(GL_NEAREST, GL_NEAREST);
	gAlbedoSpecTex->SetFilterMode(GL_NEAREST, GL_NEAREST);

	GFrameBuffer->SetColorAttachment(0, gPositionDepthTex);
	GFrameBuffer->SetColorAttachment(1, gNormalTex);
	GFrameBuffer->SetColorAttachment(2, gAlbedoSpecTex);
	GFrameBuffer->SetDepthStencilAttachment(DepthStencilBuffer);
	GFrameBuffer->SetDrawBuffers(3, DrawBuffers);
	GFrameBuffer->CheckStatus();

	// Create SSAO Calculate Frame Buffer
	FOpenGLFrameBufferRef SSAOFrameBuffer = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef SSAOTex = GLDriver.CreateTexture2D(GL_RED, screenWidth, screenHeight, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	SSAOTex->SetFilterMode(GL_NEAREST, GL_NEAREST);
	SSAOFrameBuffer->SetColorAttachment(0, SSAOTex);
	SSAOFrameBuffer->CheckStatus();

	// Create SSAO Blur FrameBuffer;
	FOpenGLFrameBufferRef SSAOBlurFrameBuffer = GLDriver.CreateFrameBuffer();
	FOpenGLTexture2DRef SSAOBlurTex = GLDriver.CreateTexture2D(GL_RED, screenWidth, screenHeight, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	SSAOBlurTex->SetFilterMode(GL_NEAREST, GL_NEAREST);
	SSAOBlurFrameBuffer->SetColorAttachment(0, SSAOBlurTex);
	SSAOBlurFrameBuffer->CheckStatus();

	//生成场景物体位置信息
	std::vector<glm::vec3> objectPositions;
	objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));

	// 计算半球采样点
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (GLuint i = 0; i < 64; ++i)
	{
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		GLfloat scale = GLfloat(i) / 64.0;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// 切空间扰动, Noise texture
	std::vector<glm::vec3> ssaoNoise;
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	FOpenGLTexture2DRef	NoiseTex = GLDriver.CreateTexture2D(GL_RGB, 4, 4, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	NoiseTex->SetFilterMode(GL_NEAREST, GL_NEAREST);
	NoiseTex->SetWrapMode(GL_REPEAT, GL_REPEAT);

	// Set Shader
	AmbientOcclusion_Shader->gPositionDepthTex = gPositionDepthTex;
	AmbientOcclusion_Shader->gNormalTex = gNormalTex;
	AmbientOcclusion_Shader->kernelSamples = ssaoKernel;
	AmbientOcclusion_Shader->texNoiseTex = NoiseTex;

	AmbientOcclusionBlur_Shader->ssaoInputTex = SSAOTex;

	SSAOLighting_Shader->gPositionDepthTex = gPositionDepthTex;
	SSAOLighting_Shader->gNormalTex = gNormalTex;
	SSAOLighting_Shader->gAlbedoSpecTex = gAlbedoSpecTex;
	SSAOLighting_Shader->ssaoFactorsTex = SSAOBlurTex;
	SSAOLighting_Shader->lightColor = lightColor;
	SSAOLighting_Shader->lightPosition = lightPos;

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

		glEnable(GL_DEPTH_TEST);
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
			viewContext.model = glm::translate(glm::mat4(), glm::vec3(0.0f, -1.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
			if (IsValidRef(Floor))
			{
				Floor->Draw(viewContext, GeometryPolicy);
			}
		}

		glDisable(GL_DEPTH_TEST);
		// PASS 2: Calculate Ambient Occlusion Factors
		if(1)
		{
			GLDriver.SetFrameBuffer(SSAOFrameBuffer);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT);

			glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

			AmbientOcclusion_Shader->projection = projection;
			QuadHelper.Draw((FCalculateAmbientOcclusionShaderType *)AmbientOcclusion_Shader, FOpenGLTexture2DRef());

			GLDriver.CheckError(__FILE__, __LINE__);
		}

		// PASS 3: Blur The Ambient Occlusion Factors
		if(1)
		{
			GLDriver.SetFrameBuffer(SSAOBlurFrameBuffer);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT);

			QuadHelper.Draw((FAmbientOcclusionBlurShaderType *)AmbientOcclusionBlur_Shader, FOpenGLTexture2DRef());
			GLDriver.CheckError(__FILE__, __LINE__);
		}

		// PASS 4: Final Lighting Pass
		if(1)
		{
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			glViewport(0, 0, screenWidth, screenHeight);
			GLDriver.SetClearColor(0.f, 0.f, 0.f, 1.0f);
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
			glm::vec3 lightViewPos = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0f));
			SSAOLighting_Shader->draw_mode = CurrentDebugCategory;
			SSAOLighting_Shader->lightPosition = lightViewPos;

			QuadHelper.Draw((FSSAOLightingShaderType *)SSAOLighting_Shader, FOpenGLTexture2DRef());
		}

		// DEBUG: Draw Lights Cube
		if (1)
		{
			glEnable(GL_DEPTH_TEST);

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
				DrawLightBox_Shader->LightColor = lightColor;

				glm::mat4 Pos = glm::translate(glm::mat4(), lightPos); // Translate it down a bit so it's at the center of the scene
				viewContext.model = glm::scale(Pos, glm::vec3(0.2f, 0.2f, 0.2f));

				DrawLightBox_Shader->LightColor = lightColor;
				LightCube->Draw(viewContext, DrawBoxPolicy);

			}
		}

		if (0)
		{
			GLDriver.SetFrameBuffer(FOpenGLFrameBufferRef());
			GLDriver.ClearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			QuadHelper.Draw(DebugNormalQuad_Shader, SSAOBlurTex);
			GLDriver.CheckError(__FILE__, __LINE__);
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
