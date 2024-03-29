#pragma once

#include <iostream>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Basic.h"
#include "ShaderMaker.h"
#include "GeomUtils.h"

enum TipoNemico{Piccolo, Medio, Grande};
static float DimP = 15.0;
static float DimM = 30.0;
static float DimG = 45.0;
static int PuntiP = 300;
static int PuntiM = 200;
static int PuntiG = 100;

static double offset_angolo = 20;
static float animDelay = 0.05;

static int nemiciStart = 30;
static int maxNemici = 60;
static float spawnDelay = 5;
static int nemiciSpawn = 3;

static int nTriangles = 30;
static int nVertices_Nemico = 12 * 3 * nTriangles + 6;

struct Nemico {
    Point* punti = new Point[nVertices_Nemico];
    Point pos;
    TipoNemico t;
};

struct HitInfo {
    bool hit;
    TipoNemico t;
    Point pos;
    int score;
};

void disegnaNemico(Color colBordoCupola, Color color_top2, Color colCentroCupola, Color colDiscoSopra, Color colBase, Point* PuntiNemico);

bool controlloDistanza(Point p, float tolleranza);

HitInfo checkHit(float xMouse, float yMouse);

bool isNemiciEmpty();

int getSizeNemici();

void aggiungiNemici(int num);

void initNemici ();

void updateNemAnim();

void disegnaNemici();