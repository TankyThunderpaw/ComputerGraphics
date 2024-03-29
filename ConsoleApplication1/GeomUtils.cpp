#include "GeomUtils.h"

float dist(Point a, Point b) {
    return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2));
}

float degtorad(float angle) {
    return angle * PI / 180;
}

float Lerp(float t, float a, float b) {
    return (1-t)*a + t*b;
}
