#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <cmath>
#include <algorithm>

#include "Basic.h"
#include "Nemico.h"
#include "Background.h"
#include "Stelle.h"
#include "PartSystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static unsigned int programId;

unsigned int MatProj;

using namespace glm;

int punteggio = 0;

/* Prototypes */
int main(int argc, char** argv);

mat4 Projection;

void framebuffer_size_callback(GLFWwindow* window, int Width, int Height) {
    height = (Height > 1) ? Height : 2;
    width = (Width > 1 ) ? Width : 2;
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouseFunc(GLFWwindow* window, int button, int action, int mods) {
    double xMouse, yMouse;
    glfwGetCursorPos(window, &xMouse, &yMouse);
    yMouse = height - yMouse;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        HitInfo check = checkHit(xMouse, yMouse);

        if (check.hit) {
            std::cout << "Colpito nemico!" << std::endl;
            punteggio += check.score;
            std::cout << "Punteggio: " << punteggio << std::endl;

            if (isNemiciEmpty()) {
                float avg = punteggio / glfwGetTime();
                std::cout << "VITTORIA! Punti al secondo: " << avg << std::endl;
                glfwSetWindowShouldClose(window, true);
            }
            else {
                // particles
                PartType type;
                switch (check.t) {
                    case Piccolo: {
                        type = P1;
                        break;
                    }
                    case Medio: {
                        type = P2;
                        break;
                    }
                    case Grande: {
                        type = P3;
                        break;
                    }
                }
                addReleasePoint(check.pos, type, 150, 0.22, 0.02);
            }
        }
	}
}

void initShader(void) {
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShaderCM.glsl";
    char* fragmentShader = (char*)"fragmentShaderCM.glsl";
    programId = ShaderMaker::createProgram(vertexShader, fragmentShader);

    glUseProgram(programId);
}

void init() {
    // seed random
    srand (static_cast <unsigned> (time(0)));

    // Inizializzazione Background
    initBackground();

    // Inizializzazione Stelle
    initStelle();

    // Inizializzazione Nemici
    initNemici();
    aggiungiNemici(nemiciStart);

    // Inizializzazione Particle System
    initParticles();

    Projection = ortho(0.0f, float(width), 0.0f, float(height));
    MatProj = glGetUniformLocation(programId, "Projection");
}

void drawScene(GLFWwindow* window) {
    glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programId);

    // Disegno bg
    disegnaBackground();

    // Disegno stelle
    disegnaStelle();

    // Disegno particelle
    drawParticles();

    // Disegno nemici
    disegnaNemici();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main(int argc, char** argv) {
    GLFWwindow* window;

    if (!glfwInit()) {
        std::cout << "Error glfwInit()" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    std::cout << "I'm apple machine" << std::endl;
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, "MyGame", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glViewport(0,0,width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, processInput);
    glfwSetMouseButtonCallback(window, mouseFunc);
    glewExperimental = GL_TRUE;

    GLFWcursor* crosshairCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    glfwSetCursor(window, crosshairCursor);

    if(glewInit() != GLEW_OK) {
        std::cout << "ERROR!" << std::endl;
    }

    initShader();
    init();
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double lastUpdNem = glfwGetTime();
    double lastUpdAnim = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        drawScene(window);

        if (glfwGetTime() - lastUpdNem >= spawnDelay && getSizeNemici() + nemiciSpawn <= maxNemici) {
            aggiungiNemici(nemiciSpawn);
            lastUpdNem = glfwGetTime();
        }

        if (glfwGetTime() - lastUpdAnim >= animDelay) {
            updateNemAnim();
            lastUpdAnim = glfwGetTime();
        }
    }

    glfwTerminate();
    return 0;
}
