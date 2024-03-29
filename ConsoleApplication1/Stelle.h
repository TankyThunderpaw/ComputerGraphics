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

const int numStelleG = 80;
const int numStelleM = 150;
const int numStelleS = 250;

const int numStelleTot = numStelleS + numStelleM + numStelleG;

struct Stelle {
    Point* pt = new Point[numStelleTot];
};

void initStelle();
void disegnaStelle();