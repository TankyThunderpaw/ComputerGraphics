#include "Background.h"

unsigned int VAO_BG;
unsigned int VBO_BG;

Background background;

using namespace glm;

unsigned int bgShaderId;
unsigned int bgMatModel;
unsigned int bgProjModel;
mat4 bgModel;
mat4 bgProj;

void initBGShader(void) {
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShaderCM.glsl";
    char* fragmentShader = (char*)"fragmentShaderCM.glsl";

    bgShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(bgShaderId);
}

void drawBGElem(int size, void** data){
    glBufferData(GL_ARRAY_BUFFER, size, &data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void disegna_piano(float x, float y, float width, float height, Color color_top, Color color_bot, Point* piano) {
    piano[0].x = x;	piano[0].y = y;
    piano[0].c.r = color_bot.r; piano[0].c.g = color_bot.g; piano[0].c.b = color_bot.b; piano[0].c.a = color_bot.a;
    piano[1].x = x + width;	piano[1].y = y;
    piano[1].c.r = color_top.r; piano[1].c.g = color_top.g; piano[1].c.b = color_top.b; piano[1].c.a = color_top.a;
    piano[2].x = x + width;	piano[2].y = y + height;
    piano[2].c.r = color_bot.r; piano[2].c.g = color_bot.g; piano[2].c.b = color_bot.b; piano[2].c.a = color_bot.a;

    piano[3].x = x + width;	piano[3].y = y + height;
    piano[3].c.r = color_bot.r; piano[3].c.g = color_bot.g; piano[3].c.b = color_bot.b; piano[3].c.a = color_bot.a;
    piano[4].x = x;	piano[4].y = y + height;
    piano[4].c.r = color_top.r; piano[4].c.g = color_top.g; piano[4].c.b = color_top.b; piano[4].c.a = color_top.a;
    piano[5].x = x;	piano[5].y = y;
    piano[5].c.r = color_bot.r; piano[5].c.g = color_bot.g; piano[5].c.b = color_bot.b; piano[5].c.a = color_bot.a;
}

void initBackground() {
    initBGShader();

    Color col_bot = {0.2,0.2,0.2, 0.2};

    disegna_piano(0.0, 0.0, width, height,col_bot,col_bot, background.punti);

    glGenVertexArrays(1, &VAO_BG);
    glBindVertexArray(VAO_BG);
    glGenBuffers(1, &VBO_BG);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_BG);
    drawBGElem(vertices_space * sizeof(Point), reinterpret_cast<void **>(background.punti));

    bgMatModel = glGetUniformLocation(bgShaderId, "Model");
    bgProjModel = glGetUniformLocation(bgShaderId, "Projection");
}

void disegnaBackground() {
    bgProj = ortho(0.0f, float(width), 0.0f, float(height));

    glBindVertexArray(VAO_BG);
    bgModel = mat4(1.0);
    glUniformMatrix4fv(bgMatModel, 1, GL_FALSE, value_ptr(bgModel));
    glUniformMatrix4fv(bgProjModel, 1, GL_FALSE, value_ptr(bgProj));
    glDrawArrays(GL_TRIANGLES, 0, vertices_space);
    glBindVertexArray(0);
}