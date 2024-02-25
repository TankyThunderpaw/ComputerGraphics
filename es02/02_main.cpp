#include "commons.h"

static unsigned int programId;

unsigned int MatProj, MatModel;

glm::mat4 Projection;  //Matrice di proiezione

// Game Loop variables
float deltaTime = 0.0f;
unsigned long timeSinceStart; // milliseconds from glutInit() call

bool running = true;
bool gameOver = false;

// INIT ===================================================
static void initShader()
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"resources/shaders/vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"resources/shaders/fragmentShader_C_M.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}

static void init()
{
	Projection = glm::ortho(0.0f, float(WINDOW_WIDTH), 0.0f, float(WINDOW_HEIGHT));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");

	spawnSpaceship();
	initStars();
	initFiretrail();
	initAsteroids();
	initBullets();

	// set background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

static void endGame(int value)
{
	std::cout << "GAME OVER" << std::endl;
	glutExit();

	exit(0);
}

// UPDATE =================================================
static void update(int value)
{
	// Wait until 16ms has elapsed since last frame
	while (glutGet(GLUT_ELAPSED_TIME) - timeSinceStart <= 16);
	
	// Delta time = time elapsed from last frame (in seconds)
	deltaTime = (glutGet(GLUT_ELAPSED_TIME) - timeSinceStart) / 1000.0f;
	
	// Clamp delta time so that it doesn't exceed ~60 fps
	deltaTime = MIN(0.05f, deltaTime);

	// Update elapsed time (for next frame)
	timeSinceStart = glutGet(GLUT_ELAPSED_TIME);

	// INPUT PROCESSING -----------------------------------
	inputSpaceship();

	// UPDATE GAME LOGIC ----------------------------------
	if (running)
	{
		updateSpaceship(deltaTime);
		updateFiretrail(deltaTime);
		updateStars(deltaTime);
		
		if (!gameOver)
		{
			updateAsteroids(deltaTime);
			updateBullets(deltaTime);
		}
	}
	if (gameOver)
	{
		glutPostRedisplay();
		glutTimerFunc(3000, endGame, 0);
	}

	glutPostRedisplay();
	glutTimerFunc(UPDATE_DELAY, update, 0);
}

// DRAW ===================================================
static void drawScene()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	

	// DRAW SCENE OBJECTS ---------------------------------
	drawStars();
	drawFiretrail();
	drawAsteroids();
	drawSpaceship();
	drawBullets();

	glutSwapBuffers();
}

// MAIN ===================================================
int main(int argc, char** argv)
{
	// Print controls
	std::cout << "CONTROLS" << std::endl;
	std::cout << "  W     - move forward" << std::endl;
	std::cout << "  A/D   - rotate left/right" << std::endl;
	std::cout << "  SPACE - fire bullet" << std::endl;
	std::cout << "  P     - toggle pause game" << std::endl;
	std::cout << "  R     - reset spaceship position" << std::endl;
	std::cout << "  C     - toggle colliders" << std::endl;
	std::cout << "  B     - toggle big spaceship" << std::endl;
	std::cout << "  L     - toggle render using only lines" << std::endl;

	glutInit(&argc, argv);
	
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("2D Animation");

	// Input callbacks
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);

	// Update callback
	timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc(UPDATE_DELAY, update, 0);

	// Draw callback
	glutDisplayFunc(drawScene);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();

	return 0;
}