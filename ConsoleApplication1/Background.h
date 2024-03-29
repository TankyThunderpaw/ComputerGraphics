#pragma once

#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Basic.h"
#include "ShaderMaker.h"

static int vertices_space = 6;

struct Background {
    Point* punti = new Point[vertices_space];
};

void initBackground();

void disegnaBackground();