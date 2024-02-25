#include "figure.h"

/*
void buildCircumference(Figure* fig, Point3D center, float radius, int numSegments, ColorRGBA color)
{
	// PI * 2 = complete circle => divide by num of triangles we want to use
	float PI = 3.14159265358979323846;
	float stepA = (PI * 2) / numSegments;
	float stepB = (1 ) / diameter;

	for (int i = 0; i <= numSegments; i++)
	{
		// Extern vertices
		Point3D v0 = { center.x + radius * cos((double)i * stepA), center.y + radius * sin((double)i * stepA), 0.0f };
		fig->vertices.push_back(v0);
		fig->colors.push_back(color);
	}
}*/