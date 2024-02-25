#include "text.h"

std::vector<Text> texts;

void addLetter(Letter* let, glm::vec4 color, char letter, float offset)
{
	let->drawMode = GL_LINE_STRIP;
	switch (letter)
	{
	case ' ':
		break;
	case 'a':
	case 'A':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		let->points.push_back({ 0.5f + offset, 0.0f});
		let->points.push_back({ -0.5f + offset, 0.0f});
		break;
	case 'b':
	case 'B':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 0.875f + offset, 0.75f});
		let->points.push_back({ 0.75f + offset, 0.5f});
		let->points.push_back({ 0.5f + offset, 0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, -0.25f});
		let->points.push_back({ 0.75f + offset, -0.5f});
		let->points.push_back({ 0.875f + offset, -0.75f});
		let->points.push_back({ 1.0f + offset, -1.0f});
		let->points.push_back({ 0.875f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, -2.0f});
		break;
	case 'c':
	case 'C':
		let->points.push_back({ 1.0f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 1.0f + offset, 1.25f});
		break;
	case 'd':
	case 'D':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 1.0f + offset, -1.0f});
		let->points.push_back({ 0.875f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, -2.0f});
		break;
	case 'e':
	case 'E':
		let->points.push_back({ 1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'f':
	case 'F':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'g':
	case 'G':
		let->points.push_back({ 0.0f + offset, -0.5f});
		let->points.push_back({ 1.0f + offset, -0.5f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, -0.5f});
		let->points.push_back({ 0.65f + offset, -1.5f});
		let->points.push_back({ 0.4f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 1.0f + offset, 1.25f});
		break;
	case 'h':
	case 'H':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		break;
	case 'i':
	case 'I':
		let->points.push_back({ -0.5f + offset, -2.0f});
		let->points.push_back({ 0.5f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ -0.5f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 2.0f});
		break;
	case 'j':
	case 'J':
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, -1.0f});
		let->points.push_back({ 0.3125f + offset, -1.45f});
		let->points.push_back({ 0.125f + offset, -1.65f});
		let->points.push_back({ -0.0625f + offset, -1.75f});
		let->points.push_back({ -0.25f + offset, -2.0f});
		let->points.push_back({ -0.4375f + offset, -1.75f});
		let->points.push_back({ -0.625f + offset, -1.65f});
		let->points.push_back({ -0.8125f + offset, -1.45f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		break;
	case 'k':
	case 'K':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		break;
	case 'l':
	case 'L':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		break;
	case 'm':
	case 'M':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		break;
	case 'n':
	case 'N':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'o':
	case 'O':
		let->points.push_back({ 1.0f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 1.0f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, -1.25f});
		break;
	case 'p':
	case 'P':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 0.875f + offset, 0.75f});
		let->points.push_back({ 0.75f + offset, 0.5f});
		let->points.push_back({ 0.5f + offset, 0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		break;
	case 'q':
	case 'Q':
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 1.0f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		let->points.push_back({ 0.25f + offset, -1.0f});
		break;
	case 'r':
	case 'R':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 0.875f + offset, 0.75f});
		let->points.push_back({ 0.75f + offset, 0.5f});
		let->points.push_back({ 0.5f + offset, 0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		break;
	case 's':
	case 'S':
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.875f + offset, -1.25f});
		let->points.push_back({ 1.0f + offset, -1.0f});
		let->points.push_back({ 0.875f + offset, -0.75f});
		let->points.push_back({ 0.75f + offset, -0.5f});
		let->points.push_back({ 0.5f + offset, -0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ -0.5f + offset, 0.25f});
		let->points.push_back({ -0.75f + offset, 0.5f});
		let->points.push_back({ -0.875f + offset, 0.75f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 't':
	case 'T':
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'u':
	case 'U':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -0.75f + offset, -1.75f});
		let->points.push_back({ -0.45f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ 0.45f + offset, -2.0f});
		let->points.push_back({ 0.75f + offset, -1.75f});
		let->points.push_back({ 0.875f + offset, -1.25f});
		let->points.push_back({ 1.0f + offset, -1.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'v':
	case 'V':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'w':
	case 'W':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ -0.5f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'x':
	case 'X':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		break;
	case 'y':
	case 'Y':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		break;
	case 'z':
	case 'Z':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		break;
	case '0':
		let->points.push_back({ 1.0f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 1.0f + offset, 1.25f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		break;
	case '1':
		let->points.push_back({ -0.75f + offset, -2.0f});
		let->points.push_back({ 0.75f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ -0.85f + offset, 1.5f});
		break;
	case '2':
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ -1.0f + offset, -2.0f});
		let->points.push_back({ 1.0f + offset, -2.0f});
		break;
	case '3':
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 0.875f + offset, 0.75f});
		let->points.push_back({ 0.75f + offset, 0.5f});
		let->points.push_back({ 0.5f + offset, 0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, -0.25f});
		let->points.push_back({ 0.75f + offset, -0.5f});
		let->points.push_back({ 0.875f + offset, -0.75f});
		let->points.push_back({ 1.0f + offset, -1.0f});
		let->points.push_back({ 0.875f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		break;
	case '4':
		let->points.push_back({ -0.5f + offset, 2.0f});
		let->points.push_back({ -0.75f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, 2.0f});
		let->points.push_back({ 0.5f + offset, -2.0f});
		break;
	case '5':
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ -1.0f + offset, 0.0f});

		let->points.push_back({ -0.875f + offset, 0.1f});
		let->points.push_back({ -0.75f + offset, 0.2f});
		let->points.push_back({ -0.5f + offset, 0.3f});
		let->points.push_back({ 0.0f + offset, 0.4f});
		let->points.push_back({ 0.5f + offset, 0.2f});
		let->points.push_back({ 0.75f + offset, 0.0f});
		let->points.push_back({ 0.875f + offset, -0.2f});
		let->points.push_back({ 1.0f + offset, -0.4f});

		let->points.push_back({ 1.0f + offset, -0.6f});
		let->points.push_back({ 0.9f + offset, -0.8f});
		let->points.push_back({ 0.8f + offset, -1.0f});
		let->points.push_back({ 0.6f + offset, -1.4f});
		let->points.push_back({ 0.3f + offset, -1.7f});
		let->points.push_back({ 0.0f + offset, -1.8f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		break;
	case '6':
		let->points.push_back({ -0.9f + offset, -0.75f});
		let->points.push_back({ -0.85f + offset, -0.5f});
		let->points.push_back({ -0.5f + offset, -0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, -0.25f});
		let->points.push_back({ 0.75f + offset, -0.5f});
		let->points.push_back({ 0.875f + offset, -0.75f});
		let->points.push_back({ 1.0f + offset, -1.0f});
		let->points.push_back({ 0.875f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ -0.9f + offset, -0.75f});
		let->points.push_back({ 0.4f + offset, 2.0f});
		break;
	case '7':
		let->points.push_back({ -1.0f + offset, 2.0f});
		let->points.push_back({ 1.0f + offset, 2.0f});
		let->points.push_back({ -0.5f + offset, -2.0f});
		break;
	case '8':
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, -0.25f});
		let->points.push_back({ 0.75f + offset, -0.5f});
		let->points.push_back({ 0.875f + offset, -0.75f});
		let->points.push_back({ 1.0f + offset, -1.0f});
		let->points.push_back({ 0.875f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -1.5f});
		let->points.push_back({ 0.5f + offset, -1.75f});
		let->points.push_back({ 0.0f + offset, -2.0f});
		let->points.push_back({ -0.5f + offset, -1.75f});
		let->points.push_back({ -0.75f + offset, -1.5f});
		let->points.push_back({ -0.875f + offset, -1.25f});
		let->points.push_back({ -1.0f + offset, -1.0f});
		let->points.push_back({ -0.9f + offset, -0.75f});
		let->points.push_back({ -0.85f + offset, -0.5f});
		let->points.push_back({ -0.5f + offset, -0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, 0.25f});
		let->points.push_back({ 0.75f + offset, 0.5f});
		let->points.push_back({ 0.875f + offset, 0.75f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.9f + offset, 0.75f});
		let->points.push_back({ -0.85f + offset, 0.5f});
		let->points.push_back({ -0.5f + offset, 0.25f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		break;
	case '9':
		let->points.push_back({ 0.75f + offset, 0.5f});
		let->points.push_back({ 0.875f + offset, 0.75f});
		let->points.push_back({ 1.0f + offset, 1.0f});
		let->points.push_back({ 0.875f + offset, 1.25f});
		let->points.push_back({ 0.75f + offset, 1.5f});
		let->points.push_back({ 0.5f + offset, 1.75f});
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ -0.5f + offset, 1.75f});
		let->points.push_back({ -0.75f + offset, 1.5f});
		let->points.push_back({ -0.875f + offset, 1.25f});
		let->points.push_back({ -1.0f + offset, 1.0f});
		let->points.push_back({ -0.9f + offset, 0.75f});
		let->points.push_back({ -0.85f + offset, 0.5f});
		let->points.push_back({ -0.5f + offset, 0.25f});
		let->points.push_back({ 0.5f + offset, 0.25f});
		let->points.push_back({ 0.75f + offset, 0.5f});
		let->points.push_back({ -0.6f + offset, -2.0f});
		break;
	case ',':
		let->points.push_back({ -0.5f + offset, -2.0f});
		let->points.push_back({ -0.8f + offset, -2.8f});
		break;
	case '.':
		let->drawMode = GL_POINTS;
		let->points.push_back({ -0.5f + offset, -2.0f});
		break;
	case ':':
		let->drawMode = GL_POINTS;
		let->points.push_back({ -0.5f + offset, 1.0f});
		let->points.push_back({ -0.5f + offset, -1.0f});
		break;
	case '(':
		let->points.push_back({ 0.0f + offset, -2.0f });
		let->points.push_back({ -0.5f + offset, -1.75f });
		let->points.push_back({ -0.75f + offset, -1.5f });
		let->points.push_back({ -0.875f + offset, -1.25f });
		let->points.push_back({ -1.0f + offset, -1.0f });
		let->points.push_back({ -1.0f + offset, 1.0f });
		let->points.push_back({ -0.875f + offset, 1.25f });
		let->points.push_back({ -0.75f + offset, 1.5f });
		let->points.push_back({ -0.5f + offset, 1.75f });
		let->points.push_back({ 0.0f + offset, 2.0f });
		break;
	case ')':
		let->points.push_back({ 0.0f + offset, 2.0f });
		let->points.push_back({ 0.5f + offset, 1.75f });
		let->points.push_back({ 0.75f + offset, 1.5f });
		let->points.push_back({ 1.0f + offset, 1.25f });
		let->points.push_back({ 1.0f + offset, -1.25f });
		let->points.push_back({ 1.0f + offset, -1.25f });
		let->points.push_back({ 0.75f + offset, -1.5f });
		let->points.push_back({ 0.5f + offset, -1.75f });
		let->points.push_back({ 0.0f + offset, -2.0f });
		break;
	case '/':
		let->points.push_back({ -0.75f + offset, -2.5f});
		let->points.push_back({ 0.75f + offset, 2.5f});
		break;
	case '\\':
		let->points.push_back({ 0.75f + offset, -2.5f});
		let->points.push_back({ -0.75f + offset, 2.5f});
		break;
	case '+':
		let->points.push_back({ -1.0f + offset, 0.0f});
		let->points.push_back({ 1.0f + offset, 0.0f});
		let->points.push_back({ 0.0f + offset, 0.0f});
		let->points.push_back({ 0.0f + offset, 1.0f});
		let->points.push_back({ 0.0f + offset, -1.0f});
		break;
	case '\'':
		let->points.push_back({ 0.0f + offset, 2.0f});
		let->points.push_back({ 0.0f + offset, 1.0f});		
		break;
	case '%':
		let->drawMode = GL_LINES;
		let->points.push_back({ -1.25f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 2.0f});
		let->points.push_back({ -0.75f + offset, 2.0f});
		let->points.push_back({ -0.25f + offset, 1.25f});
		let->points.push_back({ -0.25f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, 0.5f});
		let->points.push_back({ -0.75f + offset, 0.5f});
		let->points.push_back({ -1.25f + offset, 1.25f});
		let->points.push_back({ -0.75f + offset, -2.5f});
		let->points.push_back({ 0.75f + offset, 2.5f});
		let->points.push_back({ 1.25f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -2.0f});
		let->points.push_back({ 0.75f + offset, -2.0f});
		let->points.push_back({ 0.25f + offset, -1.25f});
		let->points.push_back({ 0.25f + offset, -1.25f});
		let->points.push_back({ 0.75f + offset, -0.5f});
		let->points.push_back({ 0.75f + offset, -0.5f});
		let->points.push_back({ 1.25f + offset, -1.25f});
		break;
	case '>':
	case '<':
	case '=':
	case '-':
	default:
		let->points.push_back({ -0.5f + offset, 0.0f});
		let->points.push_back({ 0.5f + offset, 0.0f});
		break;
	}

	for (int i = 0; i < let->points.size(); i++)
	{
		let->colors.push_back(color);
	}

}

Text createText(float posx, float posy, bool center, float scale, bool visibility, const char* message)
{
	float textWidth = strlen(message) * 1.35f * scale;

	Text text = {};
	text.pos.x = center ? posx - textWidth/2 : posx;
	text.pos.y = posy;
	text.scale = scale;
	text.visible = visibility;

	for (int i = 0; i < strlen(message); i++)
	{
		Letter letter = {};
		letter.widthLines = scale / 3;
		letter.sizePoints = scale / 1.5f;
		addLetter(&letter, { 1.0f, 1.0f, 1.0f, 1.0f }, message[i], i * 3);

		glGenVertexArrays(1, &letter.VAO);
		glBindVertexArray(letter.VAO);
		// Genero, rendo attivo, riempio il VBO della geometria dei vertici
		glGenBuffers(1, &letter.VBO_Geom);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Geom);
		// Genero, rendo attivo, riempio il VBO dei colori
		glGenBuffers(1, &letter.VBO_Col);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Col);

		text.letters.push_back(letter);
	}

	return text;
}

Text createText2(float posx, float posy, bool center, float scale, bool visibility, const char* message, glm::vec4 colour)
{
	float textWidth = strlen(message) * 1.35f * scale;

	Text text = {};
	text.pos.x = center ? posx - textWidth / 2 : posx;
	text.pos.y = posy;
	text.scale = scale;
	text.visible = visibility;

	for (int i = 0; i < strlen(message); i++)
	{
		Letter letter = {};
		letter.widthLines = scale / 3;
		letter.sizePoints = scale / 1.5f;
		addLetter(&letter, colour, message[i], i * 3);

		glGenVertexArrays(1, &letter.VAO);
		glBindVertexArray(letter.VAO);
		// Genero, rendo attivo, riempio il VBO della geometria dei vertici
		glGenBuffers(1, &letter.VBO_Geom);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Geom);
		// Genero, rendo attivo, riempio il VBO dei colori
		glGenBuffers(1, &letter.VBO_Col);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Col);

		text.letters.push_back(letter);
	}

	return text;
}

void updateText(Text* text, char* newMessage)
{
	float widthLines = text->letters.at(0).widthLines;
	float sizePoints = text->letters.at(0).sizePoints;
	text->letters.clear();

	for (int i = 0; i < strlen(newMessage); i++)
	{
		Letter letter = {};
		letter.widthLines = widthLines;
		letter.sizePoints = sizePoints;
		addLetter(&letter, { 1.0f, 1.0f, 1.0f, 1.0f }, newMessage[i], i * 3);

		glGenVertexArrays(1, &letter.VAO);
		glBindVertexArray(letter.VAO);
		// Genero, rendo attivo, riempio il VBO della geometria dei vertici
		glGenBuffers(1, &letter.VBO_Geom);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Geom);
		// Genero, rendo attivo, riempio il VBO dei colori
		glGenBuffers(1, &letter.VBO_Col);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Col);

		text->letters.push_back(letter);
	}
}

void updateText2(Text* text, glm::vec4 colour, float scale, char* newMessage)
{
	float widthLines;
	float sizePoints;
	if (scale == -1)
	{
		widthLines = text->letters.at(0).widthLines;
		sizePoints = text->letters.at(0).sizePoints;
	}
	else
	{
		widthLines = scale / 3;
		sizePoints = scale / 1.5f;
	}
	text->letters.clear();

	for (int i = 0; i < strlen(newMessage); i++)
	{
		Letter letter = {};
		letter.widthLines = widthLines;
		letter.sizePoints = sizePoints;
		addLetter(&letter, colour, newMessage[i], i * 3);

		glGenVertexArrays(1, &letter.VAO);
		glBindVertexArray(letter.VAO);
		// Genero, rendo attivo, riempio il VBO della geometria dei vertici
		glGenBuffers(1, &letter.VBO_Geom);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Geom);
		// Genero, rendo attivo, riempio il VBO dei colori
		glGenBuffers(1, &letter.VBO_Col);
		glBindBuffer(GL_ARRAY_BUFFER, letter.VBO_Col);

		text->letters.push_back(letter);
	}
}
