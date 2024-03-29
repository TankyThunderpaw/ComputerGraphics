#include "PartSystem.h"

unsigned int VAO_P1, VAO_P2, VAO_P3;
unsigned int VBO_P1, VBO_P2, VBO_P3;

vector<Particle> Particles;
vector<ParticleReleasePoint> ReleasePoints;

Particle partP1, partP2, partP3;

static float DimP1 = 2.0;
static float DimP2 = 2.5;
static float DimP3 = 4.0;

unsigned int partShaderId;
unsigned int partMatModel;
glm::mat4 partModel;

void initPartShader(void) {
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShaderCM.glsl";
    char* fragmentShader = (char*)"fragmentShaderCM.glsl";

    partShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(partShaderId);
}

void drawPartElem(int size, void** data){
    glBufferData(GL_ARRAY_BUFFER, size, &data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void disCerchio(float cx, float cy, float raggiox, float raggioy, Color color_top, Color color_bot, Point* Cerchio) {
    float stepA = (2 * PI) / nTrianglesPart;

    int comp = 0;
    for (int i = 0; i < nTrianglesPart; i++) {
        Cerchio[comp].x = cx + cos((double)i * stepA) * raggiox;
        Cerchio[comp].y = cy + sin((double)i * stepA) * raggioy;
        Cerchio[comp].c.r = color_top.r; Cerchio[comp].c.g = color_top.g; Cerchio[comp].c.b = color_top.b; Cerchio[comp].c.a = color_top.a;

        Cerchio[comp + 1].x = cx + cos((double)(i + 1) * stepA) * raggiox;
        Cerchio[comp + 1].y = cy + sin((double)(i + 1) * stepA) * raggioy;
        Cerchio[comp + 1].c.r = color_top.r; Cerchio[comp + 1].c.g = color_top.g; Cerchio[comp + 1].c.b = color_top.b; Cerchio[comp + 1].c.a = color_top.a;

        Cerchio[comp + 2].x = cx;
        Cerchio[comp + 2].y = cy;
        Cerchio[comp + 2].c.r = color_bot.r; Cerchio[comp + 2].c.g = color_bot.g; Cerchio[comp + 2].c.b = color_bot.b; Cerchio[comp + 2].c.a = color_bot.a;

        comp += 3;
    }
}

void addReleasePoint(Point point, PartType type, int partNumb, float releaseDuration, float releaseFrequency) {
    double time = glfwGetTime();

    ReleasePoints.push_back({point, type, partNumb, releaseDuration, releaseFrequency, time, time});
}

void creaParticella(Color color_top, Color color_bot, Point* PuntiPart) {
    disCerchio(0.0, 0.0, 1.0, 1.0, color_top, color_bot, PuntiPart);
}

void releaseParticles(Point inputPoint, int partNumb, PartType type) {
    float angle = 0;
    float angleStep = 2 * PI / partNumb;
    for (int i = 0; i < partNumb; i++) {
        Particle par;

        par.pos.x = inputPoint.x;
        par.pos.y = inputPoint.y;

        par.type = type;
        if (type == P1)
            par.drag = 1.025;
        else if (type == P2)
            par.drag = 1.015;
        else
            par.drag = 1.01;

        par.life = 1.0;

        par.xFactor = 1 * cos(angle);
        par.yFactor = 1 * sin(angle);

        Particles.push_back(par);

        angle += angleStep;
    }
}

void initParticles() {
    initPartShader();
    // seed random
    srand (static_cast <unsigned> (time(0)));

    // Disegno Particles
    Color col_viola1 =	{ 0.46,0.05,0.7, 1.0 };
    Color col_viola2 =	{ 0.7,0.05,0.5, 1.0 };

    Color col_verde1 =	{ 0.1,0.6,0.0, 1.0 };
    Color col_verde2 =	{ 0.0,0.8,0.0, 1.0 };

    Color col_arancione1 =	{ 0.9,0.4,0.05, 1.0 };
    Color col_arancione2 =	{ 0.95,0.45,0.3, 1.0 };


    creaParticella(col_viola1, col_viola2, partP1.points);
    glGenVertexArrays(1, &VAO_P1);
    glBindVertexArray(VAO_P1);
    glGenBuffers(1, &VBO_P1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_P1);
    drawPartElem(nTrianglesPart * sizeof(Point), reinterpret_cast<void **>(partP1.points));

    creaParticella(col_verde1, col_verde2, partP2.points);
    glGenVertexArrays(1, &VAO_P2);
    glBindVertexArray(VAO_P2);
    glGenBuffers(1, &VBO_P2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_P2);
    drawPartElem(nTrianglesPart * sizeof(Point), reinterpret_cast<void **>(partP2.points));

    creaParticella(col_arancione1, col_arancione2, partP3.points);
    glGenVertexArrays(1, &VAO_P3);
    glBindVertexArray(VAO_P3);
    glGenBuffers(1, &VBO_P3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_P3);
    drawPartElem(nTrianglesPart * sizeof(Point), reinterpret_cast<void **>(partP3.points));
}

void updateParticles() {
    int i;
    double time = glfwGetTime();
    // For each point we check if we have to release particles
    for (i = 0; i < ReleasePoints.size(); i++) {
        if(time <= ReleasePoints.at(i).startTime + ReleasePoints.at(i).releaseDuration) {
            if (time - ReleasePoints.at(i).lastReleaseTime >= ReleasePoints.at(i).releaseFrequency) {
                releaseParticles(ReleasePoints.at(i).point, ReleasePoints.at(i).particleNumber,
                                 ReleasePoints.at(i).type);
                ReleasePoints.at(i).lastReleaseTime = time;
            }
        } else {
            ReleasePoints.erase(ReleasePoints.begin() + i);
        }
    }

    // For each particle that is(still) alive we update the values :
    for (i = 0; i < Particles.size(); i++) {
        Particles.at(i).xFactor /= Particles.at(i).drag;
        Particles.at(i).yFactor /= Particles.at(i).drag;

        Particles.at(i).pos.x += Particles.at(i).xFactor;
        Particles.at(i).pos.y += Particles.at(i).yFactor;

        //std::cout << "p" << i << " x = " << particles.at(i).p.x << " y = " << particles.at(i).p.y<< std::endl;
        //std::cout << particles.at(i).p.c.r << particles.at(i).p.c.g << particles.at(i).p.c.b << std::endl;

        Particles.at(i).life -= 0.01;  // reduce life

        if (Particles.at(i).life <= 0.0) { // particle is dead
            Particles.erase(Particles.begin() + i);
        }
    }
}


void drawParticles(){
    updateParticles();
    partMatModel = glGetUniformLocation(partShaderId, "Model");

    for (int i = 0; i < Particles.size(); i++) {
        switch (Particles.at(i).type) {
            case P1: {
                glBindVertexArray(VAO_P1);
                partModel = glm::mat4(1.0);
                partModel = translate(partModel, glm::vec3(Particles.at(i).pos.x, Particles.at(i).pos.y, 0));
                partModel = scale(partModel, glm::vec3(DimP1 * Particles.at(i).life, DimP1 * Particles.at(i).life, 1.0));
                glUniformMatrix4fv(partMatModel, 1, GL_FALSE, value_ptr(partModel));
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawArrays(GL_TRIANGLES, 0, nTrianglesPart);
                glBindVertexArray(0);

                break;
            }
            case P2: {
                glBindVertexArray(VAO_P2);
                partModel = glm::mat4(1.0);
                partModel = translate(partModel, glm::vec3(Particles.at(i).pos.x, Particles.at(i).pos.y, 0));
                partModel = scale(partModel, glm::vec3(DimP2 * Particles.at(i).life, DimP2 * Particles.at(i).life, 1.0));
                glUniformMatrix4fv(partMatModel, 1, GL_FALSE, value_ptr(partModel));
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawArrays(GL_TRIANGLES, 0, nTrianglesPart);
                glBindVertexArray(0);

                break;
            }
            case P3: {
                glBindVertexArray(VAO_P3);
                partModel = glm::mat4(1.0);
                partModel = translate(partModel, glm::vec3(Particles.at(i).pos.x, Particles.at(i).pos.y, 0));
                partModel = scale(partModel, glm::vec3(DimP3 *Particles.at(i).life, DimP3 * Particles.at(i).life, 1.0));
                glUniformMatrix4fv(partMatModel, 1, GL_FALSE, value_ptr(partModel));
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawArrays(GL_TRIANGLES, 0, nTrianglesPart);
                glBindVertexArray(0);

                break;
            }
        }
    }
}