#pragma once

#define PI 3.14159265358979323846

struct Color {
    float r, g, b, a;
};

static Color col_bianco =	{ 1.0,1.0,1.0, 1.0 };
static Color col_nero =	{ 0.0,0.0,0.0, 1.0 };

struct Point {
    float x, y;
    Color c;
};

// Window size in pixels
static int	width = 1200;
static int	height = 800;
