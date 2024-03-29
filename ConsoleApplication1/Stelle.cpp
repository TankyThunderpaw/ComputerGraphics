#include "Stelle.h"

unsigned int VAO_S;
unsigned int VBO_S;

Stelle stelle;

using namespace glm;

unsigned int stShaderId;
unsigned int stMatModel;
unsigned int stProjModel;
mat4 stModel;
mat4 stProj;


void initSTShader(void) {
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShaderCM.glsl";
    char* fragmentShader = (char*)"fragmentShaderCM.glsl";

    stShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(stShaderId);
}

void drawSTElem(int size, void** data){
    glBufferData(GL_ARRAY_BUFFER, size, &data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void initStelle() {
    initSTShader();

    // seed random
    srand (static_cast <unsigned> (time(0)));

    for (int i = 0; i < numStelleTot; i++) {
        float LOx = 15;
        float HIx = width - 15;
        stelle.pt[i].x = LOx + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIx - LOx)));

        float LOy = 15;
        float HIy = height - 15;
        stelle.pt[i].y = LOy + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIy - LOy)));

        //std::cout << "Point " << i << " : " << stelle.pt[i].x << " - " << stelle.pt[i].y << std::endl;

        stelle.pt[i].c.r = 1.0; stelle.pt[i].c.g = 1.0; stelle.pt[i].c.b = 1.0; stelle.pt[i].c.a = 1.0;
    }

    glGenVertexArrays(1, &VAO_S);
    glBindVertexArray(VAO_S);
    glGenBuffers(1, &VBO_S);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
    drawSTElem(numStelleTot * sizeof(Point), reinterpret_cast<void **>(stelle.pt));


    stMatModel = glGetUniformLocation(stShaderId, "Model");
    stProjModel = glGetUniformLocation(stShaderId, "Projection");
}


void disegnaStelle() {
    stModel = mat4(1.0);

    stModel = translate((stModel, vec3(0, 0, 0)));
    stProj = ortho(0.0f, float(width), 0.0f, float(height));

    glUniformMatrix4fv(stMatModel, 1, GL_FALSE, value_ptr(stModel));
    glUniformMatrix4fv(stProjModel, 1, GL_FALSE, value_ptr(stProj));

    glBindVertexArray(VAO_S);

    glPointSize(4.0f);
    glDrawArrays(GL_POINTS, 0, numStelleG);
    glPointSize(2.5f);
    glDrawArrays(GL_POINTS, numStelleG, numStelleM);
    glPointSize(1.0f);
    glDrawArrays(GL_POINTS, numStelleG + numStelleM, numStelleS);
    glBindVertexArray(0);
}