#pragma warning
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct {
	int id;
	GLuint VAO;
	GLuint VBO_Geom;
	GLuint VBO_Col;
	std::vector<glm::vec2> points;
	std::vector<glm::vec4> colors;
	glm::vec2 pos;
	int drawMode;
	float sizePoints;
	float widthLines;
} Letter;

typedef struct {
	int id;
	std::vector<Letter> letters;
	glm::vec2 pos;
	float scale;
	bool visible;
} Text;

void addLetter(Letter* let, glm::vec4 color, char letter, float offset);
Text createText(float posx, float posy, bool center, float scale, bool visibility, const char* message);
Text createText2(float posx, float posy, bool center, float scale, bool visibility, const char* message, glm::vec4 colour);
void updateText(Text* text, char* newMessage);
void updateText2(Text* text, glm::vec4 colour, float scale, char* newMessage);

extern unsigned int MatProj, MatModel;