// Std. Includes
#include <string>

#include "Common/UtilityHelper.h"
#include "OpenGL/OpenGLDrv.h"
#include "Scene/Camera.h"
#include "Scene/Model.h"
#include "Scene/Render.h"
#include "Scene/ShaderType.h"
#include "Scene/LinesBatch.h"


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
void Draw_Debug(FModelRef &Charactor, FModelRef &BoneModel, FLinesPatchRef &Lines, const FViewContext &viewContext, FRenderPolicy &Policy);


// Camera
FCamera camera(glm::vec3(0.0f, 0.0f, 30.0f));
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, screenWidth*0.5f, screenHeight*0.5f);

	GLDriver.DeferredInitialize();

	// Define the viewport dimensions
	glViewport(0, 0, screenWidth, screenHeight);
	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);


	// Load Shader
	TRefCountPtr<FMeshShaderType> MeshShader = new FMeshShaderType("shaders/test_model.vs", "shaders/test_model.frag");
	TRefCountPtr<FSkinningMeshShaderType> SkinMeshShader = new FSkinningMeshShaderType("shaders/skeleton_mesh.vs", "shaders/skeleton_mesh.frag");
	TRefCountPtr<FLinesShaderType> LineShader = new FLinesShaderType("shaders/draw_line.vs", "shaders/draw_line.frag");

	// Load Box
	FModelRef Cube = FModel::CreateCube("textures/toy_box_normal.png");
	Cube->InitRHI();

	// Load Model
	FModelRef Model = FModel::CreateModel("objects/MD5/Bob.md5mesh");
	assert(IsValidRef(Model));
	Model->InitRHI();
	Model->Play(0);

	// LinesBatch
	const std::vector<FSimpleVertex> &LineVertes = Model->GetNodeHierarchy()->GetDebugHierarchyLines();
	FLinesPatchRef SkeletonLines = new FLinesPatch(LineVertes.size() >> 1);
	SkeletonLines->InitRHI();
	SkeletonLines->UpdateVertex(LineVertes);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		Model->Tick(deltaTime * 10.f);

		// Check and call events
		glfwPollEvents();
		Do_Movement();

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
		//viewContext.model = glm::rotate(viewContext.model, -glm::half_pi<float>(), glm::vec3(1.f, 0.f, 0.f));

		FRenderPolicy policy;
		policy.MeshShader = MeshShader;
		policy.SkinMeshShader = SkinMeshShader;
		policy.LinesShader = LineShader;

		if (IsValidRef(Model))
		{
			Model->Draw(viewContext, policy);

			//Draw_Debug(Model, Cube, SkeletonLines, viewContext, policy);
		}

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	Model->ReleaseRHI();
	// Properly de-allocate all resources once they've outlived their purpose
	glfwTerminate();
	return 0;
}

void Draw_Debug(FModelRef &Charactor, FModelRef &BoneModel, FLinesPatchRef &Lines, const FViewContext &viewContext, FRenderPolicy &Policy)
{
	FNodeHierarchyRef Skeleton = Charactor->GetNodeHierarchy();
	FViewContext ThisContext = viewContext;
	
	if (!IsValidRef(Skeleton))
	{
		std::cout << "Has No Skeleton" << std::endl;
		return;
	}
	
	const int kCount = Skeleton->GetNodesCount();
	for (int k = 0; k < kCount; k++)
	{
		const FNode& Bone = Skeleton->GetNode(k);
		ThisContext.model = viewContext.model * Bone.ModelMat;
		
		BoneModel->Draw(ThisContext, Policy);
	}

	const std::vector<FSimpleVertex> &LineVertes = Skeleton->GetDebugHierarchyLines();
	Lines->UpdateVertex(LineVertes);
	Lines->Draw(viewContext, Policy);
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

	GLfloat xoffset = lastX - xpos;
	GLfloat yoffset = ypos - lastY;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
