#include "Nemico.h"

unsigned int VAO_NP, VAO_NM, VAO_NG;
unsigned int VBO_NP, VBO_NM, VBO_NG;

vector<Nemico> Nemici;

Nemico nemicoP, nemicoM, nemicoG;

int frame_anim = 0;
float angolo = 0;

unsigned int nemShaderId;
unsigned int nemMatModel;
glm::mat4 nemModel;

bool isNemiciEmpty() {
    return Nemici.empty();
}

int getSizeNemici() {
    return Nemici.size();
}

void initNemShader(void) {
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShaderCM.glsl";
    char* fragmentShader = (char*)"fragmentShaderCM.glsl";

    nemShaderId = ShaderMaker::createProgram(vertexShader, fragmentShader);
    glUseProgram(nemShaderId);
}

void drawNemElem(int size, void** data){
    glBufferData(GL_ARRAY_BUFFER, size, &data[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void disegnaCerchio(Point center, float raggiox, float raggioy, Color color_top, Color color_bot, Point* Cerchio) {
    float stepA = (2 * PI) / nTriangles;

    int comp = 0;
    for (int i = 0; i < nTriangles; i++) {
        Cerchio[comp].x = center.x + cos((double)i * stepA) * raggiox;
        Cerchio[comp].y = center.y + sin((double)i * stepA) * raggioy;
        Cerchio[comp].c.r = color_top.r; Cerchio[comp].c.g = color_top.g; Cerchio[comp].c.b = color_top.b; Cerchio[comp].c.a = color_top.a;

        Cerchio[comp + 1].x = center.x + cos((double)(i + 1) * stepA) * raggiox;
        Cerchio[comp + 1].y = center.y + sin((double)(i + 1) * stepA) * raggioy;
        Cerchio[comp + 1].c.r = color_top.r; Cerchio[comp + 1].c.g = color_top.g; Cerchio[comp + 1].c.b = color_top.b; Cerchio[comp + 1].c.a = color_top.a;

        Cerchio[comp + 2].x = center.x;
        Cerchio[comp + 2].y = center.y;
        Cerchio[comp + 2].c.r = color_bot.r; Cerchio[comp + 2].c.g = color_bot.g; Cerchio[comp + 2].c.b = color_bot.b; Cerchio[comp + 2].c.a = color_bot.a;

        comp += 3;
    }
}

void disegnaRettangolo(Point centro, float width, float height, Color color_top, Color color_bot, Point* piano) {
    piano[0].x = centro.x - width/2;	piano[0].y = centro.y - height/2;
    piano[0].c.r = color_bot.r; piano[0].c.g = color_bot.g; piano[0].c.b = color_bot.b; piano[0].c.a = color_bot.a;
    piano[1].x =  centro.x + width/2;	piano[1].y = centro.y - height/2;
    piano[1].c.r = color_top.r; piano[1].c.g = color_top.g; piano[1].c.b = color_top.b; piano[1].c.a = color_top.a;
    piano[2].x =  centro.x + width/2;	piano[2].y =  centro.y + height/2;
    piano[2].c.r = color_bot.r; piano[2].c.g = color_bot.g; piano[2].c.b = color_bot.b; piano[2].c.a = color_bot.a;

    piano[3].x = centro.x + width/2;	piano[3].y =  centro.y + height/2;
    piano[3].c.r = color_bot.r; piano[3].c.g = color_bot.g; piano[3].c.b = color_bot.b; piano[3].c.a = color_bot.a;
    piano[4].x = centro.x - width/2;	piano[4].y =  centro.y + height/2;
    piano[4].c.r = color_top.r; piano[4].c.g = color_top.g; piano[4].c.b = color_top.b; piano[4].c.a = color_top.a;
    piano[5].x = centro.x - width/2;	piano[5].y = centro.y - height/2;
    piano[5].c.r = color_bot.r; piano[5].c.g = color_bot.g; piano[5].c.b = color_bot.b; piano[5].c.a = color_bot.a;
}

void disegnaSemicerchio(Point center, float raggiox, float raggioy, float start, Color color_top, Color color_bot, Point* Cerchio) {
    float stepA = PI / nTriangles;

    int comp = 0;
    for (int i = 0; i < nTriangles; i++) {
        Cerchio[comp].x = center.x + cos((double)i * stepA + start) * raggiox;
        Cerchio[comp].y = center.y + sin((double)i * stepA + start) * raggioy;
        Cerchio[comp].c.r = color_top.r; Cerchio[comp].c.g = color_top.g; Cerchio[comp].c.b = color_top.b; Cerchio[comp].c.a = color_top.a;

        Cerchio[comp + 1].x = center.x + cos((double)(i + 1) * stepA + start) * raggiox;
        Cerchio[comp + 1].y = center.y + sin((double)(i + 1) * stepA + start) * raggioy;
        Cerchio[comp + 1].c.r = color_top.r; Cerchio[comp + 1].c.g = color_top.g; Cerchio[comp + 1].c.b = color_top.b; Cerchio[comp + 1].c.a = color_top.a;

        Cerchio[comp + 2].x = center.x;
        Cerchio[comp + 2].y = center.y;
        Cerchio[comp + 2].c.r = color_bot.r; Cerchio[comp + 2].c.g = color_bot.g; Cerchio[comp + 2].c.b = color_bot.b; Cerchio[comp + 2].c.a = color_bot.a;

        comp += 3;
    }
}

void disegnaArco(Point center, float raggiox, float raggioy, float start, float angle, Color color_top, Color color_bot, Point* Cerchio) {
    float stepA = angle / nTriangles;

    int comp = 0;
    for (int i = 0; i < nTriangles; i++) {
        Cerchio[comp].x = center.x + cos((double)i * stepA + start) * raggiox;
        Cerchio[comp].y = center.y + sin((double)i * stepA + start) * raggioy;
        Cerchio[comp].c.r = color_top.r; Cerchio[comp].c.g = color_top.g; Cerchio[comp].c.b = color_top.b; Cerchio[comp].c.a = color_top.a;

        Cerchio[comp + 1].x = center.x + cos((double)(i + 1) * stepA + start) * raggiox;
        Cerchio[comp + 1].y = center.y + sin((double)(i + 1) * stepA + start) * raggioy;
        Cerchio[comp + 1].c.r = color_top.r; Cerchio[comp + 1].c.g = color_top.g; Cerchio[comp + 1].c.b = color_top.b; Cerchio[comp + 1].c.a = color_top.a;

        Cerchio[comp + 2].x = center.x;
        Cerchio[comp + 2].y = center.y;
        Cerchio[comp + 2].c.r = color_bot.r; Cerchio[comp + 2].c.g = color_bot.g; Cerchio[comp + 2].c.b = color_bot.b; Cerchio[comp + 2].c.a = color_bot.a;

        comp += 3;
    }
}

Color blendColor(Color c1, Color c2, float c1Rate) {
    Color res;
    res.r = c1.r * c1Rate + c2.r * (1-c1Rate);
    res.g = c1.g * c1Rate + c2.g * (1-c1Rate);
    res.b = c1.b * c1Rate + c2.b * (1-c1Rate);
    res.a = 1.0;

    return res;
}

void disegnaNemico(Color colBordoCupola, Color colCentroCupola, Color colDiscoSopra, Color colDiscoBordo, Color colBase, Point* PuntiNemico) {
    int i, cont;
    int v_temp = 3 * nTriangles;
    Point* Temp = new Point[v_temp];

    Color col_rosso =	{ 0.6,0.1,0.1, 1.0 };

    Color colRiflessoCupola = blendColor(colDiscoSopra, colBordoCupola, 0.25);

    // Semicerchio disco top
    disegnaSemicerchio({0.0, 0.0}, 1.5, 0.4, 0, colDiscoSopra, colRiflessoCupola, PuntiNemico);
    cont = 3 * nTriangles;

    // Semicerchio top cupola
    disegnaSemicerchio({0.0, 0.0}, 1, 1, 0, colBordoCupola, colCentroCupola, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    // Semicerchio colorato base
    disegnaSemicerchio({0.0, -0.75}, 0.5, .2, PI, colBordoCupola, colBordoCupola, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    // Semicerchio bot base
    disegnaSemicerchio({0.0, -0.25}, 1.2, 0.65, PI, colBase, colBase, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    // Rettangolo Bordo
    disegnaRettangolo({0.0, -0.05}, 3.0, 0.10, colDiscoBordo, colDiscoBordo, Temp);
    for (i = 0; i < 6; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 6;

    // Semicerchio Bordo
    disegnaSemicerchio({0.0, -0.10}, 1.5, 0.65, PI, colDiscoBordo, colDiscoBordo, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    // Semicerchio disco bot
    disegnaSemicerchio({0.0, 0.0}, 1.5, 0.65, PI, colDiscoSopra, colRiflessoCupola, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    // Semicerchio bot cupola
    disegnaSemicerchio({0.0, 0.0}, 1, 0.35, PI, colBordoCupola, colCentroCupola, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    // Cerchi sul disco
    disegnaCerchio({-1.225, 0.0}, 0.15, 0.07, colCentroCupola, col_bianco, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    disegnaCerchio({1.225, 0.0}, 0.15, 0.07, colCentroCupola, col_bianco, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    disegnaCerchio({-0.80, -0.365}, 0.15, 0.07, colCentroCupola, col_bianco, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    disegnaCerchio({0.80, -0.365}, 0.15, 0.07, colCentroCupola, col_bianco, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;

    disegnaCerchio({0.0, -0.5}, 0.15, 0.07, colCentroCupola, col_bianco, Temp);
    for (i = 0; i < v_temp; i++)
        PuntiNemico[i + cont] = Temp[i];
    cont += 3 * nTriangles;
    //std::cout << "Cont: " << cont << "/" << nVertices_Nemico << std::endl;

}

bool controlloDistanza(Point p, float tolleranza){
    for(int j = 0; j < Nemici.size(); j++) {
        float dim;
        switch (Nemici.at(j).t) {
            case Piccolo: {
                dim = DimP;
                break;
            }
            case Medio: {
                dim = DimM;
                break;
            }
            case Grande: {
                dim = DimG;
                break;
            }
        }
        if (dist(Nemici.at(j).pos, p) < tolleranza + dim*1.4)
            return true;
    }
    return false;
}

int getPunti(TipoNemico tipo) {
    switch (tipo) {
        case Piccolo: {
            return PuntiP;
        }
        case Medio: {
            return PuntiM;
        }
        case Grande: {
            return PuntiG;
        }
    }
    return 0;
}

HitInfo checkHit(float xMouse, float yMouse){
    HitInfo res = {false};
    for (int i = 0; i < Nemici.size() && !res.hit; i++){
        float size;
        switch (Nemici.at(i).t) {
            case Piccolo: {
                size = DimP;
                break;
            }
            case Medio: {
                size = DimM;
                break;
            }
            case Grande: {
                size = DimG;
                break;
            }
        }
        if (dist({static_cast<float>(xMouse), static_cast<float>(yMouse)}, Nemici.at(i).pos) <= size*1.4) {
            res.hit = true;
            res.t = Nemici.at(i).t;
            res.pos = Nemici.at(i).pos;
            res.score = getPunti(res.t);

            Nemici.erase(Nemici.begin()+i);
        }
    }
    return res;
}

void aggiungiNemici(int num) {
    for (int i = 0; i < num; i++) {
        Nemico n;
        float tol;
        float x, y;

        switch (rand()%3) {
            case 0: {
                n.t = Piccolo;
                tol = DimP;
                //std::cout << "Nemico Piccolo aggiunto!" << std::endl;
                break;
            }
            case 1: {
                n.t = Medio;
                tol = DimM;
                //std::cout << "Nemico Medio aggiunto!" << std::endl;
                break;
            }
            case 2: {
                n.t = Grande;
                tol = DimG;
                //std::cout << "Nemico Grande aggiunto!" << std::endl;
                break;
            }
        }

        tol = tol*1.6;
        do {
            float LOx = tol;
            float HIx = width - tol;
            x = LOx + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIx - LOx)));

            float LOy = tol;
            float HIy = height - tol;
            y = LOy + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HIy - LOy)));
        } while (controlloDistanza({x, y}, tol));

        n.pos.x = x;
        n.pos.y = y;

        Nemici.push_back(n);
    }
}

void initNemici () {
    initNemShader();
    // seed random
    srand (static_cast <unsigned> (time(0)));

    // Disegno Nemici
    Color col_rosso =	{ 0.6,0.1,0.1, 1.0 };

    Color col_viola1 =	{ 0.46,0.05,0.7, 1.0 };
    Color col_viola2 =	{ 0.7,0.05,0.5, 1.0 };
    Color col_viola3 =	{ 0.3,0.05,0.7, 1.0 };

    Color col_verde1 =	{ 0.0,0.8,0.0, 1.0 };
    Color col_verde2 =	{ 0.2,0.4,0.0, 1.0 };
    Color col_verde3 =	{ 0.1,0.6,0.0, 1.0 };

    Color col_arancione1 =	{ 0.95,0.75,0.40, 1.0 };
    Color col_arancione2 =	{ 0.9,0.4,0.05, 1.0 };
    Color col_arancione3 =	{ 0.95,0.45,0.3, 1.0 };

    Color col_grigio1 = {0.45, 0.45, 0.45, 1.0};
    Color col_grigio2 = {0.3, 0.3, 0.3, 1.0};
    Color col_grigio3 = {0.15, 0.15, 0.15, 1.0};


    disegnaNemico(col_viola3, col_viola2, col_grigio1, col_grigio2, col_grigio3, nemicoP.punti);
    glGenVertexArrays(1, &VAO_NP);
    glBindVertexArray(VAO_NP);
    glGenBuffers(1, &VBO_NP);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NP);
    drawNemElem(nVertices_Nemico * sizeof(Point), reinterpret_cast<void **>(nemicoP.punti));

    disegnaNemico(col_verde2, col_verde1, col_grigio1, col_grigio2, col_grigio3, nemicoM.punti);
    glGenVertexArrays(1, &VAO_NM);
    glBindVertexArray(VAO_NM);
    glGenBuffers(1, &VBO_NM);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NM);
    drawNemElem(nVertices_Nemico * sizeof(Point), reinterpret_cast<void **>(nemicoM.punti));

    disegnaNemico(col_arancione2, col_arancione1, col_grigio1, col_grigio2, col_grigio3, nemicoG.punti);
    glGenVertexArrays(1, &VAO_NG);
    glBindVertexArray(VAO_NG);
    glGenBuffers(1, &VBO_NG);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_NG);
    drawNemElem(nVertices_Nemico * sizeof(Point), reinterpret_cast<void **>(nemicoG.punti));
}

void updateNemAnim(){
    frame_anim += 6;
    if (frame_anim >= 360) {
        frame_anim -= 360;
    }
    angolo = cos(degtorad(frame_anim)) * offset_angolo;
}


void disegnaNemici(){
    nemMatModel = glGetUniformLocation(nemShaderId, "Model");

    for (int i = 0; i < Nemici.size(); i++) {
        switch (Nemici.at(i).t) {
            case Piccolo: {
                glBindVertexArray(VAO_NP);
                nemModel = glm::mat4(1.0);
                nemModel = translate(nemModel, glm::vec3(Nemici.at(i).pos.x, Nemici.at(i).pos.y, 0));
                nemModel = scale(nemModel, glm::vec3(DimP, DimP, 1.0));
                nemModel = rotate(nemModel, glm::radians(angolo), glm::vec3(0.0, 0.0, 1.0));
                glUniformMatrix4fv(nemMatModel, 1, GL_FALSE, value_ptr(nemModel));
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawArrays(GL_TRIANGLES, 0, nVertices_Nemico);

                break;
            }
            case Medio: {
                glBindVertexArray(VAO_NM);
                nemModel = glm::mat4(1.0);
                nemModel = translate(nemModel, glm::vec3(Nemici.at(i).pos.x, Nemici.at(i).pos.y, 0));
                nemModel = scale(nemModel, glm::vec3(DimM, DimM, 1.0));
                nemModel = rotate(nemModel, glm::radians(angolo), glm::vec3(0.0, 0.0, 1.0));
                glUniformMatrix4fv(nemMatModel, 1, GL_FALSE, value_ptr(nemModel));
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawArrays(GL_TRIANGLES, 0, nVertices_Nemico);

                break;
            }
            case Grande: {
                glBindVertexArray(VAO_NG);
                nemModel = glm::mat4(1.0);
                nemModel = translate(nemModel, glm::vec3(Nemici.at(i).pos.x, Nemici.at(i).pos.y, 0));
                nemModel = scale(nemModel, glm::vec3(DimG, DimG, 1.0));
                nemModel = rotate(nemModel, glm::radians(angolo), glm::vec3(0.0, 0.0, 1.0));
                glUniformMatrix4fv(nemMatModel, 1, GL_FALSE, value_ptr(nemModel));
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDrawArrays(GL_TRIANGLES, 0, nVertices_Nemico);

                break;
            }
        }
    }
}