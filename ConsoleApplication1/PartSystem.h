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

#include <GLFW/glfw3.h>

static int nTrianglesPart = 4;
static int nVerticesPart = nTrianglesPart + 2 * nTrianglesPart;

static float defaultReleaseDuration = 0.1;
static float defaultReleaseFrequency = 0.02;

enum PartType{P1, P2, P3};

struct Particle {
    Point* points = new Point[nVerticesPart];
    Point pos;
    float life;
    float xFactor;
    float yFactor;
    float drag;
    PartType type;
};

struct ParticleReleasePoint {
    Point point;
    PartType type;
    int particleNumber;
    float releaseDuration; // in seconds
    float releaseFrequency; // in seconds
    double startTime;
    double lastReleaseTime;
};

void initParticles();

void drawParticles();

void addReleasePoint(Point point, PartType type, int partNumb, float releaseDuration = defaultReleaseDuration, float releaseFrequency = defaultReleaseFrequency);

void releaseParticles(Point inputPoint, int partNumb, PartType type);