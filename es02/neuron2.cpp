#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <vector>

static unsigned int programId;
#define PI 3.14159265358979323846
#define MAXPR 10 // numero massimo di proiettili presenti contemporaneamente
#define TYPES 4  // numero tipi di nemici diversi

unsigned int VAO, VAO_SPACE, VAO_PROIETTILI;
unsigned int VBO, VBO_C, VBO_P, loc, MatProj, MatModel;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

void updateProiettili(int v);
void updateNemici();

vec4 col_bianco = { 1.0,1.0,1.0, 1.0 };
vec4 col_rosso = { 1.0,0.0,0.0, 1.0 };
vec4 col_nero = { 0.0,0.0,0.0, 1.0 };
vec4 col_magenta = { 1.0,1.0,0.0, 1.0 };

int NumeroColpiti = 0;
mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento da Sistema diriferimento dell'oggetto OCS a sistema Mondo WCS
typedef struct { float x, y, r, g, b, a; } Point;

//contatore frame debug
int c = 0;

// cooldown proiettili
int CD = 10;
int cooldown = CD;

int vertices_space = 6;
Point* Space = new Point[vertices_space];


// Viewport size
int width = 1280;
int height = 720;


// keys
bool pressing_left = false;
bool pressing_right = false;
bool pressing_up = false;
bool pressing_down = false;
bool pressing_attack = false;
vec2 mouseInput = vec2(0.0, 0.0);


// velocità
typedef struct {
	float x, y, z; //x: velocità in ascissa; y: velocità in ordinata; z: angolo
} Velocita;


// Navicella
typedef struct {
	GLuint VAO;
	GLuint VBO_Geom;	// VBO vertices geometry
	GLuint VBO_Col;		// VBO vertices colors
	vector<Point> points;
	vec2 pos;
	vec2 vel;
	float scale;
	float angle;
} MyNavicella;

MyNavicella player;
int num_segmenti = 30;


// Nemici
typedef struct {
	int id;
	GLuint VAO;
	GLuint VBO_Geom;
	GLuint VBO_Col;
	vector<Point> points;
	vec2 pos;
	vec2 vel;
	float scale;
	float angle;
} Nemico;

std::vector<Nemico> nemici;
vec4 colori[TYPES];
float raggi[TYPES] = { 0.8f, 0.8f, 0.8f, 0.8f };


// Proiettili
typedef struct {
	vec2 pos;  // A
	vec2 pos2; // B
	vec2 vel;
	float angle;
	bool flag; // indica se il proiettile è ancora attivo
} Proiettile;

std::vector<Proiettile> proiettili;
Proiettile listaProiettili[MAXPR];
vec2 disegnaProiettili[MAXPR * 2];
int contatoreProiettili = 0;
float velocitaProiettili = 20.0;
float lunghezzaProiettile = 20.0;
vec2 testSegmenti[2];


float lerp(float a, float b, float t) {
	//Interpolazione lineare tra a e b secondo amount
	return (1 - t) * a + t * b;
}

// restituisce l'angolo verso cui dev'essere orientata la testina della navicella
// player.pos, mouse.pos
float getAngle(int x1, int y1, int x2, int y2)
{
	float angle = atan2(y2 - y1, x2 - x1) * (180 / PI);
	return angle >= 0 ? angle : 360 + angle;
}

double  degtorad(double angle) {
	return angle * PI / 180;
}

void update(int a)
{
	float timeValue = glutGet(GLUT_ELAPSED_TIME);

	updateProiettili(0);
	updateNemici();

	// check collisione proiettile|nemici

	//aggiornamento velocità
	if (pressing_down)
	{
		if (player.vel.y > -3.0)
			player.vel.y -= 0.5;
		player.pos.y -= 10.0;
	}
	if (pressing_up)
	{
		player.pos.y += 10.0;
	}
	if (pressing_left)
	{
		player.pos.x -= 10.0;
	}
	if (pressing_right)
	{
		player.pos.x += 10.0;
	}
	/*
	//aggiornamento posizione
	player.pos.x = player.pos.x * player.vel.x;
	player.pos.y = player.pos.y * player.vel.y;
	*/

	vec2 borderSep;
	borderSep.x = 20;
	borderSep.y = borderSep.x;

	if (player.pos.x < borderSep.x)
		player.pos.x = borderSep.x;
	if (player.pos.x > width - borderSep.x)
		player.pos.x = width - borderSep.x;
	if (player.pos.y < borderSep.y)
		player.pos.y = borderSep.y;
	if (player.pos.y > height - borderSep.y)
		player.pos.y = height - borderSep.y;

	// updateProiettili(0);

	/*
	if (c%80 == 0) {
		std::cout << "Mouse(" << mouseInput.x << "," << mouseInput.y << ")" << std::endl;
		std::cout << "Player(" << player.pos.x << "," << player.pos.y << ")" << std::endl;
		std::cout << "Angle" << player.angle << std::endl << std::endl;
	}*/
	player.angle = getAngle(player.pos.x, player.pos.y, mouseInput.x, mouseInput.y) * 0.0174533;

	c++;

	glutPostRedisplay();
	glutTimerFunc(1, update, 0);
}

void onWindowResize(int w, int h) {
	glutReshapeWindow(1280, 720);
	/*
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	*/
}

void sparaProiettile(float x, float y)
{
	std::cout << "ho sparato";
	Proiettile p = {};
	float compX = cos(player.angle);
	float compY = sin(player.angle);
	p.pos = { player.pos.x, player.pos.y };
	p.pos2 = { compX * lunghezzaProiettile + p.pos.x, compY * lunghezzaProiettile + p.pos.y };
	p.vel = { compX * velocitaProiettili, compY * velocitaProiettili };
	p.angle = player.angle;
	std::cout << "\nangolo: " << p.angle << "; velX: " << p.vel.x << "; velY: " << p.vel.y;
	p.flag = true;

	proiettili.push_back(p);
	listaProiettili[contatoreProiettili] = p;
	contatoreProiettili++;
	std::cout << "\ncontatore proiettili: " << contatoreProiettili << "\n";

	glutPostRedisplay();
}

void updateProiettili(int v)
{
	int riordinamento = 0;
	int daSottrarre = 0;

	for (int i = 0; i < contatoreProiettili; i++)
	{
		// if not (check collisioni con nemici) aggiorna pos:

		//aggiorno le pos dei proiettili ciascuno attraverso la propria vel
		listaProiettili[i].pos.x += listaProiettili[i].vel.x;
		listaProiettili[i].pos2.x += listaProiettili[i].vel.x;
		listaProiettili[i].pos.y += listaProiettili[i].vel.y;
		listaProiettili[i].pos2.y += listaProiettili[i].vel.y;
		if (c % 10 == 0) {
			std::cout << "proiettile " << i << " - x: " << listaProiettili[i].pos.x << "  y: " << listaProiettili[i].pos.y << "\n";
			std::cout << "proiettile " << i << " - x2: " << listaProiettili[i].pos2.x << "  y2: " << listaProiettili[i].pos2.y << "\n";
		}

		//controllo quali proiettili sono usciti dalla finestra e vanno rimossi
		if (listaProiettili[i].pos.x > width || listaProiettili[i].pos.x < 0 ||
			listaProiettili[i].pos.y > height || listaProiettili[i].pos.y < 0)
		{
			riordinamento++;
		}
		else if (riordinamento > 0)
		{
			listaProiettili[i - riordinamento] = listaProiettili[i];
		}
	}
	contatoreProiettili -= riordinamento;

	// popolamento array dei vertici per disegnare i proiettili
	for (int i = 0; i < contatoreProiettili; i++)
	{
		disegnaProiettili[i * 2] = listaProiettili[i].pos;
		disegnaProiettili[i * 2 + 1] = listaProiettili[i].pos2;
	}

	glutPostRedisplay();
}

void updateNemici() {

}

void mouseClickEvent(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		sparaProiettile(x, y);

	glutPostRedisplay();
}

void keyboardPressedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ' ':
		pressing_attack = false;
		break;
	case 'a':
		pressing_left = true;
		break;
	case 'd':
		pressing_right = true;
		break;
	case 'w':
		pressing_up = true;
		break;
	case 's':
		pressing_down = true;
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ' ':
		pressing_attack = false;
		break;
	case 'a':
		pressing_left = false;
		break;
	case 'd':
		pressing_right = false;
		break;
	case 'w':
		pressing_up = false;
		break;
	case 's':
		pressing_down = false;
		break;
	default:
		break;
	}
}

void mousePassiveMotionEvent(int x, int y)
{
	mouseInput.x = x;
	mouseInput.y = height - y;
}

void disegna_circonferenza(MyNavicella* navicella, float centro_x, float centro_y, float raggio, vec4 colore)
{
	// step è quanto è lungo il segmento (la circonferenza avrà un numero di segmenti pari a num_segmenti)
	float step = (PI * 2) / num_segmenti;

	for (int i = 0; i <= num_segmenti; i++)
	{
		// NB: Point = (x,y,r,g,b,a)

		// calcolo il punto di partenza di ciascun segmento
		Point p = {
			centro_x + raggio * cos(i * step),
			centro_y + raggio * sin(i * step),
			colore.r,
			colore.g,
			colore.b,
			colore.a,
		};

		// aggiungo il punto al vettore (push back aggiunge in coda e aumenta la dimensione di 1)
		navicella->points.push_back(p);
	}
}

void my_disegna_navicella(MyNavicella* navicella)
{
	player.points.push_back({ 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f });
	disegna_circonferenza(&player, 0.0f, 0.0f, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
	player.points.push_back({ 1.2f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f });
}

void disegna_nemico(Nemico* nemico, int type)
{
	float step = (PI * 2) / num_segmenti;
	for (int i = 0; i <= num_segmenti; i++)
	{
		// NB: Point = (x,y,r,g,b,a)

		// calcolo il punto di partenza di ciascun segmento
		Point p = {
			nemico->pos.x + raggi[type] * cos(i * step),
			nemico->pos.y + raggi[type] * sin(i * step),
			colori[type].r,
			colori[type].g,
			colori[type].b,
			colori[type].a,
		};

		// aggiungo il punto al vettore (push back aggiunge in coda e aumenta la dimensione di 1)
		nemico->points.push_back(p);
	}
}

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";
	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}

void drawNemici() {
	for (int i = 0; i < nemici.size(); i++) {

	}
}

void init(void)
{
	//Disegno SPAZIO
	glGenVertexArrays(1, &VAO_SPACE);
	glBindVertexArray(VAO_SPACE);
	glGenBuffers(1, &VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
	glBufferData(GL_ARRAY_BUFFER, vertices_space * sizeof(Point), &Space[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Genero un VAO proiettili
	glGenVertexArrays(1, &VAO_PROIETTILI);
	glBindVertexArray(VAO_PROIETTILI);
	glGenBuffers(1, &VBO_P);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_P);

	//Definisco il colore che verrà assegnato allo schermo
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	// CREO LA STRUTTURA DELLA MIA NUOVA NAVICELLA
	player = {};
	player.pos = glm::vec2(width / 2, height / 2);
	player.angle = PI / 2;
	player.scale = 20.0f;
	// disegno la mia nuova navicella
	my_disegna_navicella(&player);
	// GENERO VAO e VBO e BINDO
	glGenVertexArrays(1, &player.VAO);
	glBindVertexArray(player.VAO);
	glGenBuffers(1, &player.VBO_Geom);
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO_Geom);
	glBufferData(GL_ARRAY_BUFFER, player.points.size() * sizeof(Point), player.points.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);



	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
}

void drawScene(void)
{
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programId);

	// draw specifiche
	drawNemici();

	Model = mat4(1.0);
	Model = translate(Model, vec3(player.pos.x, player.pos.y, 0.0));
	Model = scale(Model, vec3(player.scale, player.scale, 1.0));
	Model = rotate(Model, player.angle, vec3(0.0, 0.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(player.VAO);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINE_STRIP, 0, player.points.size());
	glBindVertexArray(0);

	// disegna proiettili
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_PROIETTILI);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_P);
	glBufferData(GL_ARRAY_BUFFER, sizeof(disegnaProiettili), &disegnaProiettili[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glLineWidth(4.0f);
	glDrawArrays(GL_LINES, 0, contatoreProiettili * 2);
	glBindVertexArray(0);

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Neuron");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(onWindowResize);
	glutMouseFunc(mouseClickEvent);
	//Evento tastiera tasto premuto
	glutKeyboardFunc(keyboardPressedEvent);
	//Evento tastiera tasto rilasciato
	glutKeyboardUpFunc(keyboardReleasedEvent);
	//Evento movimento mouse
	glutPassiveMotionFunc(mousePassiveMotionEvent);
	// glutTimerFunc a timer callback is triggered in a specified number of milliseconds
	glutTimerFunc(0, update, 0);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutMainLoop();
}