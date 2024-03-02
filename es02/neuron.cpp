#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include "text.h"
#include "sound.h"

#include <vector>

// COSE CHE MANCANO: scie più sfumate verso il fondo (mettere la trasparenza progressiva nel "disegnaScia"
// aggiungere la prima perk e aggiungere 2 nemici volendo
// possibile perk: proiettili rimbalzano tra nemici (ucciso uno va verso quello più vicino al centro del nemico appena morto)
// SCRITTA GAME OVER CHE SI SCALA (RIMPICCIOLISCE) diminuendo la trasparenza e mostrando il punteggio
// NON UTILE: micro esplosioni (bomba ma più piccola) quando raccogli una bounty
//DEBUGGARE PER CAPIRE PERCHE' L'ULTIMA WAVE LAGGA (prob c'è qualcosa in giro che non viene mai svuotato)

static unsigned int programId;
#define PI 3.14159265358979323846
#define COOLDOWN 250
#define FPS 16.7 // questo valore non rappresenta i Frames per Seconds ma i millisecondi che ne determinano il valore (33.3 = 30FPS; 16.6 = 60FPS)
#define TYPES 8  // numero tipi di nemici diversi
#define RANDOM_MAX 10
#define NUM_STAGES 8
#define MAX_WAVES 10
#define GROUPS 10
#define INVULNERABILITY 250
#define LEN_SCIA 20 // lunghezza della scia
#define PERKS 6 // numero ability perks
#define FIREPOWER 4
#define HEALTH 6
#define SPEED 3
#define INIT_SPEED
#define MAX_SPEED
#define INIT_HEALTH 58
#define MAX_HEALTH 178
#define FP_ANGLE 7.5 * 0.0174533
#define MS_MAX 60000
#define SCALA 18.0f // il valore di scala iniziale e fisso
#define DODGING 1 // numero frames durata dello stato dodging
#define SOGLIA5 250 // distanza oltre la quale un nemico di tipo 5 ruota invece che andar dritto
#define COOLDOWN5 200
#define DANNI5 25
#define COOLDOWN6 300
#define MAXNEMICI 30

unsigned int VAO, VAO_SPACE;
unsigned int VBO, VBO_S, loc, MatProj, MatModel;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

vec4 col_bianco = { 1.0,1.0,1.0, 1.0 };
vec4 col_rosso = { 1.0,0.0,0.0, 1.0 };
vec4 col_nero = { 0.0,0.0,0.0, 1.0 };
vec4 col_magenta = { 1.0,1.0,0.0, 1.0 };

int NumeroColpiti = 0;
mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento da Sistema diriferimento dell'oggetto OCS a sistema Mondo WCS
typedef struct { float x, y, r, g, b, a; } Point;

//contatore frame debug, magari reset ogni 60sec?
int c = 0;
float delta_t = 0;
int oldTimeSinceStart = 0;
int pauseStartTimestamp = 0;
int deltaPause = 0;
int s_counter = 0;
int update_counter = 0;

// cooldown proiettili
bool recharging = false;
int cooldownStart = 0;

int vertices_space = 6;
Point* Space = new Point[vertices_space];


// Viewport size
int width = 1280;
int height = 720;

// roba GameOver
bool gameOver = false;
float scalaGameOver = 100.0;  //100.0
float fattoreRiduzione = 0.01;//0.01
float progressiveTranspGO = 0.0;
float progressiveTranspPunteggio = 0.01;
bool waitingBounties = false;


// keys
bool pressing_left = false;
bool pressing_right = false;
bool pressing_up = false;
bool pressing_down = false;
bool mouseLeft_down = false;
vec2 mouseInput = vec2(0.0, 0.0);


// velocità
typedef struct {
	float up, dx, dw, sx; //x: velocità in ascissa; y: velocità in ordinata; z: angolo
} Vel4;


// Gioco
typedef struct {
	int stage;
	int points;
	int score;
	int waveInitTimestamp;
	int currentStage; // contatore dello stage corrente
	int currentWave;  // contatore dell'ondata corrente
	int currentGroup;
	int stageKills;  // contatore delle kill dello stage corrente
	int waveKills;   // contatore delle kill dell'ondata corrente
	int totalStages; // numero che indica quanti stage sono stati caricati
	int fp; // livello firepower [1-5]
	int h;	// livello health [1-7]
	int s;	// livello speed [1-4]
	int lives;
	int bombs;
	float health;
	float maxHealth;
	bool perksActive[PERKS]; // array di valori boolean per le perk attivate (max definito da maxPerks)
	int unlockedPerks; // numero progressivo delle perk sbloccate
	int numActivePerks; // numero delle perk attivate al momento per il controllo con maxPerks
	int maxPerks;
	float cdmultiplier;
} Game;

Game game;

typedef struct {
	bool attiva;
	std::vector<vec2> disegnaPerk;
	std::vector<vec4> colorePerk;
} Perk;

typedef struct {
	std::vector<vec2> disegnaActivePerkSquare;
	std::vector<vec4> coloreActivePerkSquare;
} ActivePerkSquare;

std::vector<ActivePerkSquare> quadratiPerks;
GLuint VAO_PS, VBO_PS, VBO_PSC;

typedef struct {
	float time;
	int type;
	int num;
	bool spawned;
} Group;

typedef struct {
	int stage;
	int id;
	int totEnemies;
	std::vector<Group> groups;
} Wave;

typedef struct {
	int id;
	int totEnemies;
	std::vector<Wave> waves;
} Stage;

std::vector<Stage> stageFinal;

int c_stages = 0;
int c_waves = 0;
int c_groups = 0;
int totWaveEnemies = 0;
int totStageEnemies = 0;

int old_timestamp = 0;
int new_timestamp = 0;




//stages - ondate - e per ciascuna ondata i nemici appaiono dopo un certo t (le ondate invece finiscono una volta uccisi i nemici
//lo stage cambia quando arriva il nemico nuovo o quando cambia il pattern di spawn
//per ogni stage serve: id stage (l'indice esterno), numero ondate(primo valore), due array di dim MAX_ONDATE, uno per i nemici e uno per i tempi di spawn

// STAGE: cambio nemico o pattern di spawn nemici sostanziale, composto da più waves
// WAVE: uccisione di tutti i nemici di ciascun group,
// GROUP: uccisione di tutti i nemici o solo alcuni per spawn progressivo (-1 nel tempo)



int stages[NUM_STAGES][TYPES] = {
	{4, 0, 0, 0, 0, 0, 0, 0},
	{0, 3, 0, 0, 0, 0, 0, 0},
	{4, 2, 0, 0, 0, 0, 0, 0},
	{0, 6, 0, 0, 0, 0, 0, 0},
	{0, 0, 2, 0, 0, 0, 0, 0},
	{2, 4, 0, 2, 0, 0, 0, 0},
	{0, 0, 2, 3, 0, 0, 0, 0}
}; // vettore dello stage con numeri dei nemici

int costs[5];

int nemici_per_stage[NUM_STAGES]; // calcolato in automatico all'inizio dell'init


// Navicella
typedef struct {
	GLuint VAO;
	GLuint VBO_Geom;	// VBO vertices geometry
	GLuint VBO_Col;		// VBO vertices colors
	std::vector<glm::vec2> pts;
	std::vector<glm::vec4> colors;
	glm::mat4 modelMatrix;
	vec2 pos;
	vec2 nextPos;
	vec2 vel;
	Vel4 vel4; //velocità con 4 componenti
	float scale;
	float angle;
	std::vector<glm::vec2> scia; //particellare
	std::vector<glm::vec2> puntiScia;
	std::vector<glm::vec4> coloreScia;
	std::vector<glm::vec2> puntiScia2;
	std::vector<glm::vec4> coloreScia2;
	GLuint VAO_S;
	GLuint VBO_S;
	GLuint VBO_SC;
} Navicella;

//residui scie quando muore un nemico
typedef struct {
	float transparencyLevel;
	std::vector<glm::vec2> puntiScia;
	std::vector<glm::vec4> coloreScia;
} Narehate;

std::vector<Narehate> residui;

GLuint VAO_RS, VBO_RS, VBO_RSC;

Navicella player;  //test: max_vel = 15,    acc = 0.5
float nav_raggio = 0.8f;
int num_segmenti = 30;
float max_vel = 8.0f;
float vel_inc = 0.3f;
float vel_dec = 0.4f;
float acc = 0.3f; //accelerazione
float scala = 18.0f;


// Nemici
typedef struct {
	int id;
	bool flag;
	int wave;
	int type;
	int health;
	int dodging; //solo per i tipo 4
	std::vector<int> dodged; // solo per i tipo 4
	bool revolving; // solo per i tipo 5
	int cooldown; // solo per i tipo 5
	bool child; // solo per i figli dei tipo 6
	Navicella nav;
} Nemico;

int counter_nemici_4 = 0;
int counter_nemici = 0;
std::vector<Nemico> nemici;


vec4 n_colori[TYPES] = { { 0.0, 0.0, 1.0, 1.0 },
						 { 1.0, 1.0, 0.0, 1.0 },
						 { 1.0, 0.0, 1.0, 1.0 },
						 { 0.0, 1.0, 0.0, 1.0 },
						 { 0.15, 0.725, 0.674, 1.0 },
						 { 1.0, 1.0, 1.0, 1.0 },
						 { 1.0, 0.0, 1.0, 1.0 },
						 { 1.0, 1.0, 1.0, 1.0 } };
int n_danni[TYPES] = { 20, 22, 25, 25, 25, 25, 15, 25 };
float raggi[TYPES] = { 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.8f, 0.5f, 0.8f };
float velocita[TYPES] = { 4.0f, 7.0f, 10.0f, 7.0f, 7.0f, 5.0f, 4.0f, 7.0f }; // velocità di ciascuna tipologia di nemico
int lunghezzeScia[TYPES] = { 40, 40, 60, 40, 40, 40, 40, 40 };
int bounties[TYPES] = { 1, 2, 2, 3, 3, 3, 1, 3 };

bool postmortem1 = false; // morte
bool postmortem2 = false; // lampeggio
bool postmortem3 = false; // game over
int postmortem_c = 0;
bool disegnaNav = true;
int intermittenza = 5;
bool pause = false;
bool stopFlow = false;
bool showTrails = false;
bool frameSkipperBlueN = false;
float sogliaScia = 1.0;
int sequenza_bomba = 0;
int numFramesBomba = 15;
float windowDiagonal = sqrt(width * width + height * height);
bool hoverFP = false;
bool hoverH = false;
bool hoverS = false;
bool hoverB = false;
bool hoverL = false;
bool hoverC = false;
bool hoverPMM = false;
bool hoverPA = false;
bool hoverQG = false;
bool hoverP[PERKS];
bool hoverMM[8];
// int costsFirePower[FIREPOWER] = { 1, 2, 3, 4 };
int costsFirePower[FIREPOWER] = { 45, 105, 166, 214 };
// int costsHealth[HEALTH] = { 1, 1, 1, 1, 2, 3 };
int costsHealth[HEALTH] = { 40, 80, 120, 170, 230, 300 };
// int costsSpeed[SPEED] = { 0, 0, 0 };
int costsSpeed[SPEED] = { 20, 49, 101 };
int costLife = 0;
int costBomb = 0;
char* textFirePower;
char* textHealth;
char* textSpeed;
char* textBomb;
char* textLife;

vec2 padding = { 20.0f, 20.0f }; // quanto distanti dai bordi spawnano i nemici


// Proiettili
typedef struct {
	vec2 pos;	   // A
	vec2 pos2;	   // B
	vec2 pastPos;  // last B position
	vec2 vel;
	int id;
	int flag;
	float angle;
	int bounces;
	int health;
	int type;     // 0 = proiettile; 1 = ricompensa; 2 = proiettile nemico di tipo 5
} Proiettile;

std::vector<int> proiettili_verdi;
int counter_proiettili_verdi = 0;
int counter_proiettili = 0;
std::vector<Proiettile> proiettili;
std::vector<vec2> disegnaProiettili;
std::vector<vec4> coloriProiettili;
GLuint VAO_P, VBO_P, VBO_PC;
float velocitaProiettili = 30.0;
float lunghezzaProiettile = 10.0;
float scalaProiettile = 4.0;
int pixelCheckInterval = 10;
int checkTimes = (lunghezzaProiettile + velocitaProiettili) / pixelCheckInterval;

float velocitaRicompense = 10.0;
float lunghezzaRicompense = 10.0;
float scartoAngolare = 0.1;

std::vector<vec2> disegnaBomba;
std::vector<vec4> coloreBomba;
GLuint VAO_BMB, VBO_BMB, VBO_BMBC;
vec2 bombaPos;

//strutture per grafica

//croce chiusura
std::vector<vec2> disegnaX;
std::vector<vec4> coloreX;
GLuint VAO_X, VBO_X, VBO_XC;
std::vector<vec2> disegnaXL;
std::vector<vec4> coloreXL;
GLuint VAO_XL, VBO_XL, VBO_XLC;

//main menu
std::vector<vec2> disegnaMainMenu;
std::vector<vec4> coloreMainMenu;
GLuint VAO_MM, VBO_MM, VBO_MMC;
std::vector<vec2> disegnaMainMenuL;
std::vector<vec4> coloreMainMenuL;
GLuint VAO_MML, VBO_MML, VBO_MMLC;
std::vector<Text> textMainMenu;
bool mainMenu = true;

//controls window
std::vector<vec2> disegnaControls;
std::vector<vec4> coloreControls;
GLuint VAO_CW, VBO_CW, VBO_CWC;
std::vector<vec2> disegnaControlsL;
std::vector<vec4> coloreControlsL;
GLuint VAO_CWL, VBO_CWL, VBO_CWLC;
std::vector<Text> textInstructions;
bool controls = false;

//credits window
std::vector<vec2> disegnaCredits;
std::vector<vec4> coloreCredits;
GLuint VAO_C, VBO_C, VBO_CC;
std::vector<vec2> disegnaCreditsL;
std::vector<vec4> coloreCreditsL;
GLuint VAO_CL, VBO_CL, VBO_CLC;
std::vector<Text> textCredits;

//statsBar
std::vector<vec2> disegnaStatsBar;
std::vector<vec4> coloreStatsBar;
GLuint VAO_SB, VBO_SB, VBO_SBC;
std::vector<vec2> disegnaHealthBar;
std::vector<vec4> coloreHealthBar;
GLuint VAO_HB, VBO_HB, VBO_HBC;

//menu Window
std::vector<vec2> disegnaBackgroundMenu;
std::vector<vec4> coloreBackgroundMenu;
GLuint VAO_M, VBO_M, VBO_MC;
std::vector<vec2> disegnaContornoMenu;
std::vector<vec4> coloreContornoMenu;
GLuint VAO_CM, VBO_CM, VBO_CMC;
std::vector<vec2> disegnaLucchetto;
std::vector<vec4> coloreLucchetto;
GLuint VAO_L, VBO_L, VBO_LC;
std::vector<vec2> disegnaArcoLucchetto;
std::vector<vec4> coloreArcoLucchetto;
GLuint VAO_AL, VBO_AL, VBO_ALC;
std::vector<vec2> disegnaPerks;
std::vector<vec4> colorePerks;
GLuint VAO_DP, VBO_DP, VBO_DPC;
std::vector<vec2> disegnaPerksT;
std::vector<vec4> colorePerksT;
GLuint VAO_DPT, VBO_DPT, VBO_DPTC;
std::vector<vec2> disegnaBox;
std::vector<vec4> coloreBox;
GLuint VAO_B, VBO_B, VBO_BC;
std::vector<vec2> disegnaHover;
std::vector<vec4> coloreHover;
GLuint VAO_H, VBO_H, VBO_HC;
std::vector<vec2> disegnaHoverL;
std::vector<vec4> coloreHoverL;
GLuint VAO_HL, VBO_HL, VBO_HLC;
std::vector<Text> textMenu;
std::vector<Text> textStatsBar;
Text textGameOver;
std::vector<Text> textPunteggio;
char buffer[32];
std::vector<vec2> disegnaBottoniGO;
std::vector<vec4> coloreBottoniGO;
GLuint VAO_BGO, VBO_BGO, VBO_BGOC;
std::vector<vec2> disegnaContornoBGO;
std::vector<vec4> coloreContornoBGO;
GLuint VAO_CBGO, VBO_CBGO, VBO_CBGOC;

std::vector<vec2> testingPoints;
std::vector<vec4> testingColore;
GLuint VAO_TP, VBO_TP, VBO_TPC;

float getDistance(vec2 p1, vec2 p2);
void updateProiettili();
void updateNemici();
void sparaProiettile(int i, float x, float y);
void disegnaCirconferenza(std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 coloursIxt, vec2 center, float radius, int numSeg, float degreeRange, float degreeOffset, bool type);
void disegnaCoronaCircolare(std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec2 center, vec4 coloursIxt, vec4 coloursEnt, float inRadius, float exRadius, int numTrian, float degreeRange, float degreeOffset);
void updateScie();
void disegnaScia(std::vector<vec2> scia, std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colours, float bigRadius, float lowRadius, int numTrian, Vel4 vel);
void disegnaLineeScia(std::vector<vec2> scia, std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colours, float bigRadius, float lowRadius, int numTrian, Vel4 vel);
void updateHealthBar(float newVal);
void initNemici(int num, int wave, int type);
void fuoco();
void endOfGame();

// fare una microesplosione quando i proiettili colpiscono o vengono raccolti o quando si colpisce un nemico
// creando un vettore che per ciascuna esplosione tiene un oggetto esplosione contenente posizione del centro e livello di progresso
// a ogni update se la size() è diversa da zero aumenta il livello di progresso, se questo esce dal max viene rimosso l'oggetto
// nella drawscene viene disegnato costantemente il contenuto se ha size > 0

char* intToCharBuff(int val)
{
	std::string str = std::to_string(val);
	char* buff = new char[str.length() + 1];
	strcpy_s(buff, str.length() + 1, str.c_str());
	return buff;
}

char* stringToCharBuff(string str)
{
	char* buff = new char[str.length() + 1];
	strcpy_s(buff, str.length() + 1, str.c_str());
	return buff;
}

void initPerks()
{
	int x = width * 210 / 1000;
	int y = height * 420 / 1000;
	disegnaCirconferenza(&disegnaPerks, &colorePerks, { 0.0, 0.0, 0.0, 1.0 }, { x, y }, nav_raggio, num_segmenti, 2 * PI, 0, false);
	disegnaCirconferenza(&disegnaPerks, &colorePerks, { 0.0, 0.0, 0.0, 1.0 }, { x, y }, nav_raggio / 2, num_segmenti, 2 * PI, 0, true);

}

void disegnaTriangolo(std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colour, float x1, float x2, float x3, float y1, float y2)
{
	int size = resPts->size();
	resPts->push_back({ x1, y1 });
	resPts->push_back({ x2, y1 });
	resPts->push_back({ x3, y2 });

	for (int i = size; i < resPts->size(); i++)
	{
		resCol->push_back(colour);
	}
}

void disegnaRettangolo(std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colour, float x1, float x2, float y1, float y2)
{
	int size = resPts->size();
	resPts->push_back({ x1, y2 });
	resPts->push_back({ x2, y2 });
	resPts->push_back({ x2, y1 });
	resPts->push_back({ x2, y1 });
	resPts->push_back({ x1, y2 });
	resPts->push_back({ x1, y1 });

	for (int i = size; i < resPts->size(); i++)
	{
		resCol->push_back(colour);
	}
}

void update_dodged(int id) // funzione da chiamare quando un proiettile viene eliminato, per toglierne l'ID dalla lista dodged
{						   // dei nemici di tipo 4, al fine di non saturare il vettore e velocizzare la ricerca
	int j = 0;
	for (int i = 0; i < nemici.size() && j < counter_nemici_4; i++)
	{
		if (nemici.at(i).type == 4)
		{
			int trovato = -1;
			for (int k = 0; k < nemici.at(i).dodged.size(); k++)
				if (nemici.at(i).dodged.at(k) == id)
					trovato = k;
			if (trovato != -1)
				nemici.at(i).dodged.erase(nemici.at(i).dodged.begin() + trovato);
			j++;
		}
	}
}

bool search_dodged(int id_n, int id_p)	// funzione per controllare se un proiettile con un particolare id_p è già stato dodgiato
{										// dal nemici id_n, in tal caso non deve dodgiarlo di nuovo (max 1 dodge per proiettile)
	for (int i = 0; i < nemici.at(id_n).dodged.size(); i++)
	{
		if (nemici.at(id_n).dodged.at(i) == id_p)
			return true;
	}
	return false;
}

void updateHealthBar(float newVal)
{
	disegnaHealthBar.clear();

	disegnaHealthBar.push_back({ width * 572 / 1000, height * 966 / 1000 });
	disegnaHealthBar.push_back({ width * (572 + newVal) / 1000, height * 966 / 1000 });
	disegnaHealthBar.push_back({ width * (572 + newVal) / 1000, height * 987 / 1000 });

	disegnaHealthBar.push_back({ width * (572 + newVal) / 1000, height * 987 / 1000 });
	disegnaHealthBar.push_back({ width * 572 / 1000, height * 966 / 1000 });
	disegnaHealthBar.push_back({ width * 572 / 1000, height * 987 / 1000 });
}

void quadratiPerkAttiveInit()
{
	int x1 = width * 170 / 1000;
	int x2 = width * 250 / 1000;
	int y1 = height * 355 / 1000;
	int y2 = height * 485 / 1000;
	for (int i = 0; i < PERKS / 2; i++)
	{
		ActivePerkSquare aps = {};
		disegnaRettangolo(&aps.disegnaActivePerkSquare, &aps.coloreActivePerkSquare, { 0.46f, 0.86f, 0.46f, 1.0f }, x1 + (i * 130), x2 + (i * 130), y1, y2);
		quadratiPerks.push_back(aps);
	}
	y1 -= 120;
	y2 -= 120;
	for (int i = 0; i < PERKS / 2; i++)
	{
		ActivePerkSquare aps = {};
		disegnaRettangolo(&aps.disegnaActivePerkSquare, &aps.coloreActivePerkSquare, { 0.46f, 0.86f, 0.46f, 1.0f }, x1 + (i * 130), x2 + (i * 130), y1, y2);
		quadratiPerks.push_back(aps);
	}
}

void printVec2(std::vector<vec2> v)
{
	for (int i = 0; i < v.size(); i++)
	{
		std::cout << "element " << i << ":  (" << v.at(i).x << ", " << v.at(i).y << ")\n";
	}
}

void printVec4(std::vector<vec4> v)
{
	for (int i = 0; i < v.size(); i++)
	{
		std::cout << "element 1:  (" << v.at(i).r << "," << v.at(i).g
			<< "," << v.at(i).b << "," << v.at(i).a << ")\n";
	}
}

float lerp(float a, float b, float t) {
	//Interpolazione lineare tra a e b secondo amount
	return (1 - t) * a + t * b;
}

// restituisce l'angolo verso cui dev'essere orientata la testina della navicella
// player.pos, mouse.pos
float getAngle(vec2 p1, vec2 p2) // restituisce l'angolo della retta passante per due punti
{
	float angle = atan2(p2.y - p1.y, p2.x - p1.x) * (180 / PI);
	return angle >= 0 ? angle : 360 + angle;
}

float getDistance(vec2 p1, vec2 p2)
{
	return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

float getDistancePointToLine(vec2 p1, vec2 p2, vec2 p3) // p1 = punto, p2|p3 = retta
{
	float a = p2.y - p3.y;
	float b = p3.x - p2.x;
	float c = (p2.x - p3.x) * p2.y + (p3.y - p2.y) * p2.x;
	float res = abs(a * p1.x + b * p1.y + c) / sqrt(a * a + b * b);
	return res;
}

float randZeroToOne()
{
	return (float)(rand() / (RAND_MAX + 1.));
}

float getRandomFloat(float min, float max)
{
	if (min >= max)
		return 0.0f;
	return min + (((float)rand() / (float)RAND_MAX) * (max - min));
}

bool getRandomBool()
{
	return rand() > RAND_MAX / 2;
}

double degtorad(double angle) {
	return angle * PI / 180;
}

void adjust_angle(float x, float y)
{
	if (x > y)
	{
		x += 1.0;
	}
}

void calc_nemici_per_stage() {  // popola l'array che contiene il totale dei nemici per ogni stage
	int c;
	for (int i = 0; i < NUM_STAGES; i++) {
		c = 0;
		for (int j = 0; j < TYPES; j++) {
			c += stages[i][j];
		}
		nemici_per_stage[i] = c;
	}
}

void consumePoints(int p)
{
	game.points -= p;
	updateText(&textMenu.at(1), intToCharBuff(game.points));
	updateText(&textStatsBar.at(5), intToCharBuff(game.points));
}

void vitaPersa()
{
	mouseLeft_down = false;
	if (game.lives == 1)
	{
		//Narehate r = {};				//il residuo scia del player quando muore non è molto bello
		//r.transparencyLevel = 1.0;
		//r.puntiScia = player.puntiScia;
		//r.coloreScia = player.coloreScia;
		//residui.push_back(r);
		game.lives--;
		updateText(&textStatsBar.at(1), intToCharBuff(game.lives));
		postmortem3 = true;
		disegnaNav = false;
		endOfGame();
	}
	else
	{
		//Narehate r = {};
		//r.transparencyLevel = 1.0;
		//r.puntiScia = player.puntiScia;
		//r.coloreScia = player.coloreScia;
		//residui.push_back(r);
		postmortem1 = true;
		player.scia.clear();
		player.puntiScia.clear();
		player.coloreScia.clear();
		game.lives--;
		updateText(&textStatsBar.at(1), intToCharBuff(game.lives));
		updateText(&textMenu.at(16), intToCharBuff(game.lives));
	}
}

void disegnaContornoRettangolo(std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colour, float x1, float x2, float y1, float y2)
{
	resPts->push_back({ x1, y1 });
	resPts->push_back({ x2, y1 });
	resPts->push_back({ x2, y1 });
	resPts->push_back({ x2, y2 });
	resPts->push_back({ x2, y2 });
	resPts->push_back({ x1, y2 });
	resPts->push_back({ x1, y2 });
	resPts->push_back({ x1, y1 });
	for (int i = 0; i < 8; i++)
		resCol->push_back(colour);
}

void initMainMenu()
{
	textMainMenu.push_back(createText2(width * 440 / 1000, height * 9 / 10, false, 10.0, true, "NEURON", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 482 / 1000, height * 750 / 1000, false, 5.0, true, "PLAY", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 270 / 1000, height * 598 / 1000, false, 5.0, true, "ACHIEVEMENTS", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 605 / 1000, height * 598 / 1000, false, 5.0, true, "HIGH SCORES", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 214 / 1000, height * 446 / 1000, false, 5.0, true, "INSTRUCTIONS", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 660 / 1000, height * 446 / 1000, false, 5.0, true, "WALKTHROUGH", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 280 / 1000, height * 294 / 1000, false, 5.0, true, "MORE GAMES", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 623 / 1000, height * 294 / 1000, false, 5.0, true, "DOWNLOAD", { 1.0, 1.0, 1.0, 0.7 }));
	textMainMenu.push_back(createText2(width * 460 / 1000, height * 142 / 1000, false, 5.0, true, "CREDITS", { 1.0, 1.0, 1.0, 0.7 }));
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.008, 0.059, 0.2, 1.0 }, 0, width, 0, height);
	float x1 = width * 390 / 1000;
	float x2 = width * 610 / 1000;
	float y1 = height * 690 / 1000;
	float y2 = height * 810 / 1000;
	float xInc1 = 210;
	float xInc2 = 280;
	float yInc1 = 110;
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1, x2, y1, y2);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1, x2, y1, y2);
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1 - xInc1, x2 - xInc1, y1 - yInc1, y2 - yInc1);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1 - xInc1, x2 - xInc1, y1 - yInc1, y2 - yInc1);
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1 + xInc1, x2 + xInc1, y1 - yInc1, y2 - yInc1);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1 + xInc1, x2 + xInc1, y1 - yInc1, y2 - yInc1);
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1 - xInc2, x2 - xInc2, y1 - yInc1*2, y2 - yInc1*2);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1 - xInc2, x2 - xInc2, y1 - yInc1 * 2, y2 - yInc1 * 2);
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1 + xInc2, x2 + xInc2, y1 - yInc1*2, y2 - yInc1*2);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1 + xInc2, x2 + xInc2, y1 - yInc1 * 2, y2 - yInc1 * 2);
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1 - xInc1, x2 - xInc1, y1 - yInc1*3, y2 - yInc1*3);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1 - xInc1, x2 - xInc1, y1 - yInc1 * 3, y2 - yInc1 * 3);
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1 + xInc1, x2 + xInc1, y1 - yInc1*3, y2 - yInc1*3);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1 + xInc1, x2 + xInc1, y1 - yInc1 * 3, y2 - yInc1 * 3);
	disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 0.7, 0.7 }, x1, x2, y1 - yInc1 * 4, y2 - yInc1 * 4);
	disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.7 }, x1, x2, y1 - yInc1 * 4, y2 - yInc1 * 4);
}

void creditsInit()
{

}

void instructionsInit()
{

}

void gameOverInit()
{
	//text
	textGameOver = createText2(width * 78 / 1000, height * 638 / 1000, true, scalaGameOver, true, "GAME OVER", { 1.0, 1.0, 1.0, 0.0 });
	snprintf(buffer, 32, "TOTAL SCORE: %5d", game.score);
	textPunteggio.push_back(createText2(width * 280 / 1000, height * 425 / 1000, false, 10.0, true, buffer, { 1.0, 1.0, 1.0, 0.0 }));
	textPunteggio.push_back(createText2(width * 280 / 1000, height * 215 / 1000, false, 6.0, true, "PLAY AGAIN", { 1.0, 1.0, 1.0, 0.0 }));
	textPunteggio.push_back(createText2(width * 600 / 1000, height * 215 / 1000, false, 6.0, true, "MAIN MENU", { 1.0, 1.0, 1.0, 0.0 }));
	disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.0, 1.0, 0.0, 0.0 }, width * 240 / 1000, width * 450 / 1000, height * 150 / 1000, height * 280 / 1000);
	disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 1.0, 0.0, 0.0, 0.0 }, width * 550 / 1000, width * 760 / 1000, height * 150 / 1000, height * 280 / 1000);
	disegnaContornoBGO.push_back({ width * 240 / 1000, height * 150 / 1000 });
	disegnaContornoBGO.push_back({ width * 450 / 1000, height * 150 / 1000 });
	disegnaContornoBGO.push_back({ width * 450 / 1000, height * 150 / 1000 });
	disegnaContornoBGO.push_back({ width * 450 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 450 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 240 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 240 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 240 / 1000, height * 150 / 1000 });
	disegnaContornoBGO.push_back({ width * 550 / 1000, height * 150 / 1000 });
	disegnaContornoBGO.push_back({ width * 760 / 1000, height * 150 / 1000 });
	disegnaContornoBGO.push_back({ width * 760 / 1000, height * 150 / 1000 });
	disegnaContornoBGO.push_back({ width * 760 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 760 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 550 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 550 / 1000, height * 280 / 1000 });
	disegnaContornoBGO.push_back({ width * 550 / 1000, height * 150 / 1000 });
	for (int i = 0; i < 16; i++)
		coloreContornoBGO.push_back({ 1.0, 1.0, 1.0, 0.0 });
}



void statsBarInit()
{
	//text
	Text textLives = createText(width * 10 / 1000, height * 977 / 1000, false, 5.0, true, "LIVES");
	textStatsBar.push_back(textLives);
	Text textLivesNum = createText(width * 85 / 1000, height * 977 / 1000, false, 5.0, true, intToCharBuff(game.lives));
	textStatsBar.push_back(textLivesNum);
	Text textBombs = createText(width * 140 / 1000, height * 977 / 1000, false, 5.0, true, "BOMBS");
	textStatsBar.push_back(textBombs);
	Text textBombsNum = createText(width * 213 / 1000, height * 977 / 1000, false, 5.0, true, intToCharBuff(game.bombs));
	textStatsBar.push_back(textBombsNum);
	Text textPoints = createText(width * 290 / 1000, height * 977 / 1000, false, 5.0, true, "POINTS");
	textStatsBar.push_back(textPoints);
	Text textPointsNum = createText(width * 375 / 1000, height * 977 / 1000, false, 5.0, true, intToCharBuff(game.points));
	textStatsBar.push_back(textPointsNum);
	Text textHealth = createText(width * 490 / 1000, height * 977 / 1000, false, 5.0, true, "HEALTH");
	textStatsBar.push_back(textHealth);
	Text textScore = createText(width * 820 / 1000, height * 977 / 1000, false, 5.0, true, "SCORE");
	textStatsBar.push_back(textScore);
	Text textScoreNum = createText(width * 893 / 1000, height * 977 / 1000, false, 5.0, true, intToCharBuff(game.score));
	textStatsBar.push_back(textScoreNum);

	disegnaRettangolo(&disegnaStatsBar, &coloreStatsBar, { 1.0f, 1.0f, 1.0f, 0.2f }, 0.0f, width, height, height * 955 / 1000);

	int maxH = 58;

	disegnaRettangolo(&disegnaHealthBar, &coloreHealthBar, { 1.0f, 1.0f, 1.0f, 1.0f },
		width * 572 / 1000, width * (572 + maxH) / 1000, height * 987 / 1000, height * 966 / 1000);
}

void menuGraficheInit()
{
	//background menu
	disegnaRettangolo(&disegnaBackgroundMenu, &coloreBackgroundMenu, { 1.0f, 1.0f, 1.0f, 0.2f },
		width * 15 / 100, width * 85 / 100, height * 10 / 100, height * 85 / 100);

	//contorno menu
	disegnaContornoRettangolo(&disegnaContornoMenu, &coloreContornoMenu, { 1.0f, 1.0f, 1.0f, 0.8f }, width * 15 / 100, width * 85 / 100, height * 10 / 100, height * 85 / 100);

	//boxes
	float x1 = width * 170 / 1000;
	float x2 = width * 250 / 1000;
	float y1 = height * 355 / 1000;
	float y2 = height * 485 / 1000;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);

	//lucchetto
	float x1_2 = width * 185 / 1000;
	float x2_2 = width * 235 / 1000;
	float y1_2 = height * 375 / 1000;
	float y2_2 = height * 422 / 1000;

	disegnaRettangolo(&disegnaLucchetto, &coloreLucchetto, { 0.0f, 0.0f, 0.0f, 0.8f }, x1_2, x2_2, y1_2, y2_2);
	disegnaCoronaCircolare(&disegnaArcoLucchetto, &coloreArcoLucchetto, { (float)(x1_2 + x2_2) / 2, (float)y2_2 }, { 0.0, 0.0, 0.0, 0.8f }, { 0.0, 0.0, 0.0, 0.8f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);

	x1 += 130;
	x2 += 130;
	x1_2 += 130;
	x2_2 += 130;


	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);
	disegnaRettangolo(&disegnaLucchetto, &coloreLucchetto, { 0.0f, 0.0f, 0.0f, 0.8f }, x1_2, x2_2, y1_2, y2_2);
	disegnaCoronaCircolare(&disegnaArcoLucchetto, &coloreArcoLucchetto, { (float)(x1_2 + x2_2) / 2, (float)y2_2 }, { 0.0, 0.0, 0.0, 0.8f }, { 0.0, 0.0, 0.0, 0.8f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);

	x1 += 130;
	x2 += 130;
	x1_2 += 130;
	x2_2 += 130;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);
	disegnaRettangolo(&disegnaLucchetto, &coloreLucchetto, { 0.0f, 0.0f, 0.0f, 0.8f }, x1_2, x2_2, y1_2, y2_2);
	disegnaCoronaCircolare(&disegnaArcoLucchetto, &coloreArcoLucchetto, { (float)(x1_2 + x2_2) / 2, (float)y2_2 }, { 0.0, 0.0, 0.0, 0.8f }, { 0.0, 0.0, 0.0, 0.8f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);

	x1 -= 260;
	x2 -= 260;
	x1_2 -= 260;
	x2_2 -= 260;
	y1 -= 120;
	y2 -= 120;
	y1_2 -= 120;
	y2_2 -= 120;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);
	disegnaRettangolo(&disegnaLucchetto, &coloreLucchetto, { 0.0f, 0.0f, 0.0f, 0.8f }, x1_2, x2_2, y1_2, y2_2);
	disegnaCoronaCircolare(&disegnaArcoLucchetto, &coloreArcoLucchetto, { (float)(x1_2 + x2_2) / 2, (float)y2_2 }, { 0.0, 0.0, 0.0, 0.8f }, { 0.0, 0.0, 0.0, 0.8f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);

	x1 += 130;
	x2 += 130;
	x1_2 += 130;
	x2_2 += 130;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);
	disegnaRettangolo(&disegnaLucchetto, &coloreLucchetto, { 0.0f, 0.0f, 0.0f, 0.8f }, x1_2, x2_2, y1_2, y2_2);
	disegnaCoronaCircolare(&disegnaArcoLucchetto, &coloreArcoLucchetto, { (float)(x1_2 + x2_2) / 2, (float)y2_2 }, { 0.0, 0.0, 0.0, 0.8f }, { 0.0, 0.0, 0.0, 0.8f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);

	x1 += 130;
	x2 += 130;
	x1_2 += 130;
	x2_2 += 130;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);
	disegnaRettangolo(&disegnaLucchetto, &coloreLucchetto, { 0.0f, 0.0f, 0.0f, 0.8f }, x1_2, x2_2, y1_2, y2_2);
	disegnaCoronaCircolare(&disegnaArcoLucchetto, &coloreArcoLucchetto, { (float)(x1_2 + x2_2) / 2, (float)y2_2 }, { 0.0, 0.0, 0.0, 0.8f }, { 0.0, 0.0, 0.0, 0.8f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);


	//disegna quadratini upgrades
	x1 = width * 440 / 1000;
	x2 = width * 458 / 1000;
	y1 = height * 682 / 1000;
	y2 = height * 714 / 1000;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);

	y1 -= 30;
	y2 -= 30;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);

	y1 -= 30;
	y2 -= 30;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);


	// disegna triangoli upgrade
	x1 = width * 442 / 1000;
	x2 = width * 456 / 1000;
	float x3 = (x1 + x2) / 2;
	y1 = height * 688 / 1000;
	y2 = height * 709 / 1000;
	disegnaTriangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, x3, y1, y2);

	y1 -= 30;
	y2 -= 30;
	disegnaTriangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, x3, y1, y2);

	y1 -= 30;
	y2 -= 30;
	disegnaTriangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, x3, y1, y2);


	// disegna quadrati buy
	x1 = width * 790 / 1000;
	x2 = width * 808 / 1000;
	y1 = height * 682 / 1000;
	y2 = height * 714 / 1000;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);

	y1 -= 30;
	y2 -= 30;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);


	// disegna simboli buy
	x1 = width * 792 / 1000;
	x2 = width * 807 / 1000;
	y1 = height * 695 / 1000;
	y2 = height * 702 / 1000;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, y1, y2);

	y1 -= 30;
	y2 -= 30;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, y1, y2);

	x1 = width * 797 / 1000;
	x2 = width * 802 / 1000;
	y1 = height * 685 / 1000;
	y2 = height * 712 / 1000;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, y1, y2);

	y1 -= 30;
	y2 -= 30;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, y1, y2);


	// disegna quadrati volume
	x1 = width * 790 / 1000;
	x2 = width * 808 / 1000;
	y1 = height * 460 / 1000;
	y2 = height * 492 / 1000; //+32
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);

	y1 -= 55;
	y2 -= 55;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0f, 1.0f, 1.0f, 0.8f }, x1, x2, y1, y2);


	// disegna simboli volume
	x1 = width * 793 / 1000;
	x2 = width * 798 / 1000;
	y1 = height * 470 / 1000; //+10^
	y2 = height * 482 / 1000; //+12
	disegnaRettangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, y1, y2);

	y1 -= 55;
	y2 -= 55;
	disegnaRettangolo(&disegnaBox, &coloreBox, { 0.0f, 0.0f, 0.0f, 0.8f }, x1, x2, y1, y2);


	x1 = width * 798 / 1000;
	x2 = width * 806 / 1000;
	y1 = height * 462 / 1000;
	y2 = height * 470 / 1000;
	float y3 = height * 482 / 1000;
	float y4 = height * 490 / 1000;
	disegnaBox.push_back({ x1, y3 });
	disegnaBox.push_back({ x2, y4 });
	disegnaBox.push_back({ x1, y2 });
	disegnaBox.push_back({ x1, y2 });
	disegnaBox.push_back({ x2, y4 });
	disegnaBox.push_back({ x2, y1 });

	disegnaBox.push_back({ x1, y3 - 55 });
	disegnaBox.push_back({ x2, y4 - 55 });
	disegnaBox.push_back({ x1, y2 - 55 });
	disegnaBox.push_back({ x1, y2 - 55 });
	disegnaBox.push_back({ x2, y4 - 55 });
	disegnaBox.push_back({ x2, y1 - 55 });

	int size = disegnaBox.size() - coloreBox.size();

	for (int i = 0; i < size; i++)
		coloreBox.push_back({ 0.0, 0.0, 0.0, 0.8 });

	//bottone Controls
	disegnaRettangolo(&disegnaBox, &coloreBox, { 0.0, 0.0, 1.0, 0.5 }, width * 650 / 1000, width * 810 / 1000, height * 250 / 1000, height * 330 / 1000);
	disegnaContornoRettangolo(&disegnaContornoMenu, &coloreContornoMenu, { 1.0, 1.0, 1.0, 0.5 }, width * 650 / 1000, width * 810 / 1000, height * 250 / 1000, height * 330 / 1000);

	//bottone MainMenu
	disegnaRettangolo(&disegnaBox, &coloreBox, { 1.0, 0.0, 0.0, 0.5 }, width * 650 / 1000, width * 810 / 1000, height * 150 / 1000, height * 230 / 1000);
	disegnaContornoRettangolo(&disegnaContornoMenu, &coloreContornoMenu, { 1.0, 1.0, 1.0, 0.5 }, width * 650 / 1000, width * 810 / 1000, height * 150 / 1000, height * 230 / 1000);
}

void menuInit()
{
	//text
	Text textMenuScore = createText(width * 200 / 1000, height * 780 / 1000, false, 4.0, true, "POINTS");
	textMenu.push_back(textMenuScore);
	Text textMenuScoreValue = createText(width * 270 / 1000, height * 780 / 1000, false, 4.0, true, "0");
	textMenu.push_back(textMenuScoreValue);
	Text textMenuLevel = createText(width * 370 / 1000, height * 780 / 1000, true, 4.0, true, "LEVEL");
	textMenu.push_back(textMenuLevel);
	Text textMenuUpgrade = createText(width * 440 / 1000, height * 780 / 1000, true, 4.0, true, "UPGRADE");
	textMenu.push_back(textMenuUpgrade);
	Text textMenuFirePower = createText(width * 200 / 1000, height * 700 / 1000, false, 4.0, true, "FIREPOWER");
	textMenu.push_back(textMenuFirePower);
	Text textMenuFirePowerLvl = createText(width * 370 / 1000, height * 700 / 1000, false, 4.0, true, "1");
	textMenu.push_back(textMenuFirePowerLvl);
	Text textMenuHealth = createText(width * 200 / 1000, height * 660 / 1000, false, 4.0, true, "HEALTH");
	textMenu.push_back(textMenuHealth);
	Text textMenuHealthLvl = createText(width * 370 / 1000, height * 660 / 1000, false, 4.0, true, "1");
	textMenu.push_back(textMenuHealthLvl);
	Text textMenuSpeed = createText(width * 200 / 1000, height * 620 / 1000, false, 4.0, true, "SPEED");
	textMenu.push_back(textMenuSpeed);
	Text textMenuSpeedLvl = createText(width * 370 / 1000, height * 620 / 1000, false, 4.0, true, "1");
	textMenu.push_back(textMenuSpeedLvl);
	Text textMenuAbilityPerks = createText(width * 310 / 1000, height * 525 / 1000, true, 6.0, true, "ABILITY PERKS");
	textMenu.push_back(textMenuAbilityPerks);
	Text textInfo = createText(width * 175 / 1000, height * 160 / 1000, false, 4.0, true, " "); //11
	textMenu.push_back(textInfo);
	Text textMenuConsumables = createText(width * 570 / 1000, height * 780 / 1000, false, 4.0, true, "CONSUMABLES");
	textMenu.push_back(textMenuConsumables);
	Text textMenuBomb = createText(width * 570 / 1000, height * 700 / 1000, false, 4.0, true, "BOMB");
	textMenu.push_back(textMenuBomb);
	Text textMenuBombNum = createText(width * 720 / 1000, height * 700 / 1000, false, 4.0, true, intToCharBuff(game.bombs));
	textMenu.push_back(textMenuBombNum);
	Text textMenuLife = createText(width * 570 / 1000, height * 660 / 1000, false, 4.0, true, "LIFE");
	textMenu.push_back(textMenuLife);
	Text textMenuLifeNum = createText(width * 720 / 1000, height * 660 / 1000, false, 4.0, true, intToCharBuff(game.lives));
	textMenu.push_back(textMenuLifeNum);
	Text textMenuQnt = createText(width * 710 / 1000, height * 780 / 1000, false, 4.0, true, "QNT.");
	textMenu.push_back(textMenuQnt);
	Text textMenuBuy = createText(width * 790 / 1000, height * 780 / 1000, false, 4.0, true, "BUY");
	textMenu.push_back(textMenuBuy);
	Text textMenuOptions = createText(width * 650 / 1000, height * 550 / 1000, true, 6.0, true, "OPTIONS");
	textMenu.push_back(textMenuOptions);
	Text textMenuMusic = createText(width * 570 / 1000, height * 475 / 1000, false, 4.0, true, "MUSIC");
	textMenu.push_back(textMenuMusic);
	Text textMenuSounds = createText(width * 570 / 1000, height * 400 / 1000, false, 4.0, true, "SOUNDS");
	textMenu.push_back(textMenuSounds);
	Text textMenuCTRL = createText2(width * 693 / 1000, height * 290 / 1000, false, 4.5, true, "CONTROLS", { 1.0, 1.0, 1.0, 0.5 });
	textMenu.push_back(textMenuCTRL);
	Text textMenuMM = createText2(width * 687 / 1000, height * 190 / 1000, false, 4.5, true, "MAIN MENU", { 1.0, 1.0, 1.0, 0.5 });
	textMenu.push_back(textMenuMM);

	menuGraficheInit();
}

void distruggiNemico(int i)
{
	Nemico n = nemici.at(i);
	if (n.type == 4)
	{
		for (int i = 0; i < bounties[n.type]; i++)
			sparaProiettile(i, n.nav.pos.x, n.nav.pos.y);
		counter_nemici_4--;
	}
	else
		for (int i = 0; i < bounties[n.type]; i++)
			sparaProiettile(i, n.nav.pos.x, n.nav.pos.y);


	Narehate r = {};

	r.transparencyLevel = 1.0;
	r.puntiScia = n.nav.puntiScia;
	r.coloreScia = n.nav.coloreScia;
	residui.push_back(r);

	nemici.erase(nemici.begin() + i);
}

void updateResidui()
{

	int max = residui.size();
	std::vector<glm::vec4> temp;
	for (int i = 0; i < max; i++)
	{
		if (residui.at(i).transparencyLevel > 0.02)
		{
			std::vector<glm::vec4> cur = residui.at(i).coloreScia;
			for (int j = 0; j < cur.size(); j++)
			{
				temp.push_back({ cur.at(j).r, cur.at(j).g, cur.at(j).b, cur.at(j).a > 0 ? (cur.at(j).a - 0.02) : 0 });
			}
			residui.at(i).transparencyLevel -= 0.02;
			residui.at(i).coloreScia = temp;
			temp.clear();
		}
		else
		{
			residui.erase(residui.begin() + i);
			i--;
			max--;
		}
	}
}

void unlockPerk()
{
	int x = width * 210 / 1000;
	int y = height * 420 / 1000;
	if (game.unlockedPerks < 7)
	{
		if (game.unlockedPerks == 1)
		{
			disegnaCirconferenza(&disegnaPerks, &colorePerks, { 0.0, 0.0, 0.0, 1.0 }, { x, y }, nav_raggio * player.scale * 2, num_segmenti, 2 * PI, 0, false);
			disegnaPerks.push_back({ x + nav_raggio * player.scale * 2, y });
			colorePerks.push_back({ 0.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x + nav_raggio * player.scale * 2 / 2, y });
			colorePerks.push_back({ 0.0, 0.0, 0.0, 1.0 });
			disegnaCirconferenza(&disegnaPerks, &colorePerks, { 0.0, 0.0, 0.0, 1.0 }, { x, y }, nav_raggio * player.scale, num_segmenti, 2 * PI, 0, true);
			disegnaPerks.push_back({ x + nav_raggio * player.scale, y });
			colorePerks.push_back({ 0.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x, y + nav_raggio * player.scale * 2 * 2 / 3 });
			colorePerks.push_back({ 0.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x, y });
			colorePerks.push_back({ 0.0, 0.0, 0.0, 1.0 });
		}
		if (game.unlockedPerks == 2)
		{
			int x1 = (width * 173 / 1000) + 130;
			int x2 = (width * 247 / 1000) + 130;
			int y1 = height * 360 / 1000;
			int y2 = height * 380 / 1000;
			int yDiv = (y - y2) / 6;
			int xDiv = (x2 - x1) / 12;
			disegnaRettangolo(&disegnaPerksT, &colorePerksT, { 0.0, 0.0, 0.0, 1.0 }, x1, x2, y1, y2);
			disegnaPerks.push_back({ x1, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + xDiv, y - yDiv });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 2), y - (yDiv * 2) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 3), y - (yDiv * 3) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 4), y - (yDiv * 4) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 5), y - (yDiv * 5) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 6), y - (yDiv * 6) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 7), y - (yDiv * 5) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 8), y - (yDiv * 4) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 9), y - (yDiv * 3) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 10), y - (yDiv * 2) });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 11), y - yDiv });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 12), y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + (xDiv * 13), y + yDiv });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
		}
		if (game.unlockedPerks == 3)
		{
			int x1 = (width * 173 / 1000) + 260;
			int x2 = (width * 247 / 1000) + 260;
			int y1 = height * 360 / 1000;
			int y2 = height * 380 / 1000;
			int yDiv = (y - y2) / 2;
			int xDiv = (x2 - x1) / 12;
			disegnaPerks.push_back({ x1, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 13, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 26, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 39, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 52, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 58, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 64, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 70, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 76, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 82, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 88, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
			disegnaPerks.push_back({ x1 + 94, y });
			colorePerks.push_back({ 1.0, 0.0, 0.0, 1.0 });
		}
		disegnaLucchetto.erase(disegnaLucchetto.begin(), disegnaLucchetto.begin() + 6);
		coloreLucchetto.erase(coloreLucchetto.begin(), coloreLucchetto.begin() + 6);
		disegnaArcoLucchetto.erase(disegnaArcoLucchetto.begin(), disegnaArcoLucchetto.begin() + 90);
		coloreArcoLucchetto.erase(coloreArcoLucchetto.begin(), coloreArcoLucchetto.begin() + 90);
	}
}

void checkCollisioni()
{
	vec2 a = player.pos; // posizione giocatore
	for (int i = 0; i < nemici.size(); i++)
	{
		vec2 b = nemici.at(i).nav.pos; // posizione nemico
		float radiusN = raggi[nemici.at(i).type] * scala;
		float radiusP = nav_raggio * player.scale;
		float distance1 = sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));

		if (sequenza_bomba > 0)
		{
			float distance2 = sqrtf((bombaPos.x - b.x) * (bombaPos.x - b.x)
				+ (bombaPos.y - b.y) * (bombaPos.y - b.y)) - radiusN;
			float curRadiusBomba = sqrtf(width * width + height * height) * sequenza_bomba / numFramesBomba;
			if (distance2 < curRadiusBomba)
			{
				playSoundEffect(1, "hit");
				if (!nemici.at(i).child)
				{
					game.waveKills++;
					game.stageKills++;
				}
				distruggiNemico(i);
			}
		}
		else {


			if (distance1 < (radiusN + radiusP)) // navicella colpita
			{
				playSoundEffect(1, "hit");
				if (!postmortem2)
				{
					for (int j = 0; j < proiettili.size(); j++) // cancella proiettili
					{
						if (proiettili.at(j).type == 0) // SOLO i proiettili (non le ricompense)
						{
							proiettili.erase(proiettili.begin() + j);
						}
					}
					if (game.health - n_danni[nemici.at(i).type] <= 0)
					{
						vitaPersa();
						game.health = 0;
						updateHealthBar(game.health);
					}
					else
						game.health -= n_danni[nemici.at(i).type];
				}
				if (!nemici.at(i).child)
				{
					game.waveKills++;
					game.stageKills++;
				}
				distruggiNemico(i);
				break;
			}
			else
			{
				for (int j = 0; j < proiettili.size(); j++)
				{
					vec2 c = proiettili.at(j).pos2;
					vec2 d = proiettili.at(j).pastPos;
					//voglio un controllo ogni 10 pixel, se lunghezzaProiettile + velocitaProiettili = 40 quindi check: 4 volte
					//bool trovato = false;
					if (proiettili.at(j).type == 0) // proiettile
					{
						if (nemici.at(i).flag == false && proiettili.at(j).flag == 0)
						{
							bool trovato = false;
							for (int k = 0; k < checkTimes && !trovato; k++)
							{
								c.x += k * proiettili.at(j).vel.x / checkTimes;
								c.y += k * proiettili.at(j).vel.y / checkTimes;
								float distance2 = sqrtf((c.x - b.x) * (c.x - b.x) + (c.y - b.y) * (c.y - b.y));
								if (distance2 < (radiusN + scalaProiettile)) //nemico colpito
								{
									Nemico n = nemici.at(i);
									playSoundEffect(1, "hit");
									if (n.type == 3 && n.health == 2)
									{
										nemici.at(i).health--;
										nemici.at(i).nav.pts.erase(nemici.at(i).nav.pts.begin() + num_segmenti + 1,
											nemici.at(i).nav.pts.end());
										nemici.at(i).nav.colors.erase(nemici.at(i).nav.colors.begin() + num_segmenti + 1,
											nemici.at(i).nav.colors.end());
									}
									else
									{
										nemici.at(i).flag = true;
										if (!nemici.at(i).child)
										{
											game.waveKills++;
											game.stageKills++;
										}
									}
									proiettili.at(j).flag++;
									trovato = true;
								}
							}
						}

					}
					else if (proiettili.at(j).type == 1)// ricompensa
					{
						float distance3 = sqrtf((a.x - c.x) * (a.x - c.x) + (a.y - c.y) * (a.y - c.y));
						if (distance3 < radiusP) //ricompensa raccolta dal giocatore
						{
							playSoundEffect(2, "bounty");
							proiettili.erase(proiettili.begin() + j);
							game.score++;
							game.points++;
							updateText(&textMenu.at(1), intToCharBuff(game.points));
							updateText(&textStatsBar.at(5), intToCharBuff(game.points));
							updateText(&textStatsBar.at(8), intToCharBuff(game.score));
						}

					}
				}
			}
		}
	}
	if (!postmortem1)
	{
		float radiusP = nav_raggio * player.scale;
		bool trovato = false;
		int id_p;
		for (int j = 0; j < proiettili.size() && !trovato; j++)
			if (proiettili.at(j).type == 2)
				if (getDistance(proiettili.at(j).pos, player.pos) < radiusP) // FINIRE COLLISIONE
				{
					playSoundEffect(1, "hit");
					trovato = true;
					proiettili.erase(proiettili.begin() + j);
					id_p = j;
				}
		if (trovato && !postmortem2)
		{
			for (int j = 0; j < proiettili.size(); j++) // cancella proiettili
			{
				if (proiettili.at(j).type == 2) // SOLO i proiettili (non le ricompense)
				{
					proiettili.erase(proiettili.begin() + j);
				}
			}
			if (game.health - DANNI5 <= 0)
			{
				vitaPersa();
				game.health = 0;
				updateHealthBar(game.health);
			}
			else
				game.health -= DANNI5;
		}
	}

	if (nemici.size() == 0)
		for (int i = 0; i < proiettili.size(); i++)
			if (proiettili.at(i).type == 1)
			{
				vec2 c = proiettili.at(i).pos2;
				float radiusP = nav_raggio * scala;
				float distance3 = sqrtf((a.x - c.x) * (a.x - c.x) + (a.y - c.y) * (a.y - c.y));
				if (distance3 < radiusP) //ricompensa raccolta dal giocatore
				{
					playSoundEffect(2, "bounty");
					proiettili.erase(proiettili.begin() + i);
					game.score++;
					game.points++;
					updateText(&textMenu.at(1), intToCharBuff(game.points));
					updateText(&textStatsBar.at(5), intToCharBuff(game.points));
					updateText(&textStatsBar.at(8), intToCharBuff(game.score));
				}
			}
}

void disegnaNemico(int indice)
{
	int t = nemici.at(indice).type;
	float step = (PI * 2) / num_segmenti;
	if (t == 4)
	{
		vec2 pb = {
			0.0f + raggi[t] * cos(0),
			0.0f + raggi[t] * sin(0)
		};
		for (int i = 1; i <= num_segmenti; i++)
		{
			// calcolo il punto di partenza di ciascun segmento
			vec2 p = {
				0.0f + raggi[t] * cos(i * step),
				0.0f + raggi[t] * sin(i * step)
			};

			// aggiungo il punto al vettore (push back aggiunge in coda e aumenta la dimensione di 1)
			nemici.at(indice).nav.pts.push_back(pb);
			nemici.at(indice).nav.colors.push_back(n_colori[t]);
			pb = p;
			nemici.at(indice).nav.pts.push_back(p);
			nemici.at(indice).nav.colors.push_back(n_colori[t]);
		}
		nemici.at(indice).nav.pts.push_back({ -raggi[t] * 1.1, 0.0 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
		nemici.at(indice).nav.pts.push_back({ -raggi[t] * 0.5, 0.0 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
		nemici.at(indice).nav.pts.push_back({ raggi[t] * 1.1, 0.0 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
		nemici.at(indice).nav.pts.push_back({ raggi[t] * 0.5, 0.0 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
		nemici.at(indice).nav.pts.push_back({ 0.0, raggi[t] * 1.1 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
		nemici.at(indice).nav.pts.push_back({ 0.0, raggi[t] * 0.5 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
		nemici.at(indice).nav.pts.push_back({ 0.0, -raggi[t] * 1.1 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
		nemici.at(indice).nav.pts.push_back({ 0.0, -raggi[t] * 0.5 });
		nemici.at(indice).nav.colors.push_back(n_colori[t]);
	}
	else
	{
		for (int i = 0; i <= num_segmenti; i++)
		{
			// calcolo il punto di partenza di ciascun segmento
			vec2 p = {
				0.0f + raggi[t] * cos(i * step),
				0.0f + raggi[t] * sin(i * step)
			};

			// aggiungo il punto al vettore (push back aggiunge in coda e aumenta la dimensione di 1)
			nemici.at(indice).nav.pts.push_back(p);
			nemici.at(indice).nav.colors.push_back(n_colori[t]);
		}
		if (t == 3)
		{
			for (int i = 0; i <= num_segmenti && nemici.at(indice).nav.pts.size() < 62; i++)
			{
				// calcolo il punto di partenza di ciascun segmento
				vec2 p = {
					0.0f + raggi[t] * 2 / 3 * cos(i * step),
					0.0f + raggi[t] * 2 / 3 * sin(i * step)
				};

				// aggiungo il punto al vettore (push back aggiunge in coda e aumenta la dimensione di 1)
				nemici.at(indice).nav.pts.push_back(p);
				nemici.at(indice).nav.colors.push_back(n_colori[t]);
			}
		}
	}
}


void initStages() {
	Stage s = {};
	Wave w = {};
	Group g = {};
	c_stages = 0;
	c_waves = 0;
	c_groups = 0;
	totWaveEnemies = 0;
	totStageEnemies = 0;

	/*g = {0.0, 1, 1, false};		// TEST
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	s.id = c_stages;
	s.totEnemies = totStageEnemies;
	stageFinal.push_back(s);
	c_stages++;
	c_waves = 0;
	totStageEnemies = 0;
	s.waves.clear();
	game.totalStages = c_stages;*/

	// S0 W0
	g = { 1.0, 0, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 2.0, 1, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 5.0, 0, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 7.0, 1, 1, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 7.5, 0, 10, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S0 W1
	g = { 1.0, 1, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 2.0, 1, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S0 W2
	g = { 0.0, 1, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 6.0, 1, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	s.id = c_stages;
	s.totEnemies = totStageEnemies;
	stageFinal.push_back(s);
	c_stages++;
	c_waves = 0;
	totStageEnemies = 0;
	s.waves.clear();

	// S1 W0
	g = { 3.0, 2, 1, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S1 W1
	g = { 3.0, 2, 1, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S1 W2
	g = { 3.0, 2, 1, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S1 W3
	g = { 3.0, 1, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S1 W4
	g = { 3.0, 2, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S1 W5
	g = { 3.0, 3, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 9.0, 0, 10, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 12.0, 3, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 14.0, 3, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 18.5, 2, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S1 W6
	g = { 2, 3, 10, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	s.id = c_stages;
	s.totEnemies = totStageEnemies;
	stageFinal.push_back(s);
	c_stages++;
	c_waves = 0;
	totStageEnemies = 0;
	s.waves.clear();

	// S2 W0
	g = { 3, 4, 1, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W1
	g = { 0.5, 4, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W2
	g = { 0.5, 4, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W3
	g = { 0.5, 4, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W3
	g = { 4.5, 5, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W4
	g = { 3, 5, 6, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 3, 1, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 3, 0, 6, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W5
	g = { 3, 5, 8, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 3, 0, 6, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W6
	g = { 3, 4, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 4.5, 0, 12, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 6.5, 1, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 6.5, 5, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 10, 4, 3, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 12, 3, 2, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W7
	g = { 3, 6, 8, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 4, 0, 8, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 6.5, 1, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 9.5, 3, 5, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	// S2 W8
	g = { 3, 1, 5, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 9.5, 6, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 10, 0, 4, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 15, 0, 8, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 15, 6, 5, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	g = { 19.5, 1, 5, false };
	w.groups.push_back(g);
	totWaveEnemies += w.groups.at(c_groups).num;
	c_groups++;
	w.id = c_waves;
	w.totEnemies = totWaveEnemies;
	w.stage = c_stages;
	s.waves.push_back(w);
	c_waves++;
	totStageEnemies += totWaveEnemies;
	c_groups = 0;
	totWaveEnemies = 0;
	w.groups.clear();

	s.id = c_stages;
	s.totEnemies = totStageEnemies;
	stageFinal.push_back(s);
	c_stages++;
	c_waves = 0;
	totStageEnemies = 0;
	s.waves.clear();
	game.totalStages = c_stages;

	//STAMPA MAPPA
	std::cout << "\n\nNUOVI STAGES:\n\n";
	for (int i = 0; i < stageFinal.size(); i++)
	{
		std::cout << "{ STAGE " << i + 1 << "   totNemici: " << stageFinal.at(i).totEnemies << "\n";
		for (int j = 0; j < stageFinal.at(i).waves.size(); j++)
		{
			std::cout << "{ wave " << j + 1 << "   totNemici: " << stageFinal.at(i).waves.at(j).totEnemies;
			for (int k = 0; k < stageFinal.at(i).waves.at(j).groups.size(); k++)
			{
				std::cout << " { " << (float)stageFinal.at(i).waves.at(j).groups.at(k).time << ", "
					<< stageFinal.at(i).waves.at(j).groups.at(k).type << ", "
					<< stageFinal.at(i).waves.at(j).groups.at(k).num;

				if (k == stageFinal.at(i).waves.at(j).groups.size() - 1)
					std::cout << " } ";
				else
					std::cout << " },";
			}
			if (j == stageFinal.at(i).waves.size() - 1)
				std::cout << " }\n";
			else
				std::cout << " },\n";
		}
		if (i == stageFinal.size() - 1)
			std::cout << "}\n\n";
		else
			std::cout << "}, ";
	}


	// STAGE: cambio nemico o pattern di spawn nemici sostanziale, composto da più waves
	// WAVE: uccisione di tutti i nemici di ciascun group,
	// GROUP: uccisione di tutti i nemici o solo alcuni per spawn progressivo (-1 nel tempo)

	vec3 stagesFixed[NUM_STAGES][MAX_WAVES][GROUPS] = {
	{     // stage1
		{ {1.0, 0, 3}, {2.0, 1, 2}, {5.0, 0, 4}, {7.0, 1, 1}, {7.5, 0, 10} }, // wave1
		{ {1.0, 1, 2}, {2.0, 1, 2} }, // wave2
		{ {0.0, 1, 3}, {6.0, 1, 2} }  // wave3
	}, {  // stage2
		{ {3.0, 2, 1} }, // wave1
		{ {3.0, 2, 1} }, // wave2
		{ {3.0, 2, 1} }, // wave3
		{ {3.0, 1, 4} }, // wave4
		{ {3.0, 2, 2} }, // wave5
		{ {3.0, 3, 3}, {9.0, 0, 10}, {18.5, 2, 3} } // wave6
	}
	};
}

void initNemici(int num, int wave, int type)
{
	float px = 0, py = 0;
	float angle = 0.0f;
	float vx = 0, vy = 0;

	for (int i = 0; i < num; i++)
	{
		Nemico n = {};
		int rand = getRandomFloat(0, 4);
		if (rand % 4 == 0) // lato sinistro
		{
			px = padding.x;
			py = getRandomFloat(padding.y, height - padding.y);
			angle = getRandomFloat(-1.2f, 1.2f);
			//if (getRandomBool())
			//	angle += 4.81f;
		}
		else if (rand % 4 == 1) // lato basso
		{
			px = getRandomFloat(padding.x, width - padding.x);
			py = padding.y;
			angle = getRandomFloat(0.3f, 2.8f);
			//if (getRandomBool())
			//	angle += 1.57f;
		}
		else if (rand % 4 == 2) // lato destro
		{
			px = width - padding.x;
			py = getRandomFloat(padding.y, height - padding.y);
			angle = getRandomFloat(1.9f, 4.4f);
			//if (getRandomBool())
			//	angle += 1.57f;
		}
		else // if (rand % 4 == 3) // lato superiore
		{
			px = getRandomFloat(padding.x, width - padding.x);
			py = height - padding.y;
			angle = getRandomFloat(3.5f, 5.9f);
			//if (getRandomBool())
			//	angle += 1.57f;
		}

		vec2 pos = { px, py };
		vec2 vel = { cos(angle) * velocita[type], sin(angle) * velocita[type] };
		n.id = i;
		n.flag = false;
		n.wave = wave;
		n.type = type;
		n.dodging = 0;
		n.nav.pos = pos;
		n.nav.vel = vel;
		n.nav.nextPos = { n.nav.pos.x + n.nav.vel.x, n.nav.pos.y + n.nav.vel.y };
		if (n.type == 3)
			n.health = 2;
		if (n.type == 4)
			counter_nemici_4++;
		if (n.type == 5)
		{
			n.cooldown = COOLDOWN5;
			n.revolving = false;
		}
		if (n.type == 6)
		{
			n.cooldown = COOLDOWN6;
		}
		n.child = false;
		nemici.push_back(n);
	}

	// DISEGNO NEMICI
	//genero VAO nemici
	for (int i = nemici.size() - num; i < nemici.size(); i++)
	{
		glGenVertexArrays(1, &nemici.at(i).nav.VAO);
		glBindVertexArray(nemici.at(i).nav.VAO);
		//vertici
		glGenBuffers(1, &nemici.at(i).nav.VBO_Geom);
		glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Geom);
		//colori
		glGenBuffers(1, &nemici.at(i).nav.VBO_Col);
		glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Col);

		nemici.at(i).nav.scale = scala;
		disegnaNemico(i); // qui inserisco quelli di ciascun nemico
		glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Geom);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Col);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);


		glGenVertexArrays(1, &nemici.at(i).nav.VAO_S);
		glBindVertexArray(nemici.at(i).nav.VAO_S);
		//vertici
		glGenBuffers(1, &nemici.at(i).nav.VBO_S);
		glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_S);
		//colori
		glGenBuffers(1, &nemici.at(i).nav.VBO_SC);
		glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_SC);

		//nemici.at(i).nav.puntiScia.push_back(nemici.at(i).nav.pos);
	}
}


void endOfGame()
{
	int finalScore = game.score + game.points;
	textPunteggio.clear();
	snprintf(buffer, 32, "TOTAL SCORE: %5d", finalScore);
	textPunteggio.push_back(createText2(width * 280 / 1000, height * 425 / 1000, false, 10.0, true, buffer, { 1.0, 1.0, 1.0, 0.0 }));
	textPunteggio.push_back(createText2(width * 280 / 1000, height * 215 / 1000, false, 6.0, true, "PLAY AGAIN", { 1.0, 1.0, 1.0, 0.0 }));
	textPunteggio.push_back(createText2(width * 600 / 1000, height * 215 / 1000, false, 6.0, true, "MAIN MENU", { 1.0, 1.0, 1.0, 0.0 }));
	gameOver = true;
	mouseLeft_down = false;
}

void reset()
{
	game.stage = 0;
	game.waveKills = 0;
	game.stageKills = 0;
	game.currentGroup = 0;
	game.currentWave = 0;
	game.currentStage = 0;
	game.score = 0;
	game.points = 0;
	game.unlockedPerks = 0;
	game.lives = 3;
	game.bombs = 2;
	game.health = 58;
	game.maxHealth = INIT_HEALTH;
	game.fp = 0;
	game.h = 0;
	game.s = 0;
	std::stringstream ss;
	ss << "UPGRADE FIREPOWER (COST " << costsFirePower[0] << ")";
	textFirePower = stringToCharBuff(ss.str());
	ss.clear();
	ss.str(std::string());
	ss << "UPGRADE HEALTH (COST " << costsHealth[0] << ")";
	textHealth = stringToCharBuff(ss.str());
	ss.clear();
	ss.str(std::string());
	ss << "UPGRADE SPEED (COST " << costsSpeed[0] << ")";
	textSpeed = stringToCharBuff(ss.str());

	game.waveInitTimestamp = glutGet(GLUT_ELAPSED_TIME);
	oldTimeSinceStart = 0;
	deltaPause = 0;
	game.cdmultiplier = 1;
	for (int i = 0; i < PERKS; i++)
		hoverP[i] = false;
	for (int i = 0; i < PERKS; i++)
		game.perksActive[i] = false;
	game.unlockedPerks = 0;
	game.numActivePerks = 0;
	game.maxPerks = 1;
	nemici.clear();
	proiettili.clear();
	residui.clear();
	gameOver = false;
	waitingBounties = false;
	pause = false;
	stopFlow = false;
	player.pos = { width / 2, height / 2 };
	player.scia.clear();


	scalaGameOver = 100.0;
	fattoreRiduzione = 0.01;
	progressiveTranspGO = 0.0;
	progressiveTranspPunteggio = 0.01;
	textGameOver = createText2(width * 78 / 1000, height * 638 / 1000, true, scalaGameOver, true, "GAME OVER", { 1.0, 1.0, 1.0, 0.0 });
	textPunteggio.clear();
	snprintf(buffer, 32, "TOTAL SCORE: %5d", game.score);
	textPunteggio.push_back(createText2(width * 280 / 1000, height * 425 / 1000, false, 10.0, true, buffer, { 1.0, 1.0, 1.0, 0.0 }));
	textPunteggio.push_back(createText2(width * 280 / 1000, height * 215 / 1000, false, 6.0, true, "PLAY AGAIN", { 1.0, 1.0, 1.0, 0.0 }));
	textPunteggio.push_back(createText2(width * 600 / 1000, height * 215 / 1000, false, 6.0, true, "MAIN MENU", { 1.0, 1.0, 1.0, 0.0 }));
	disegnaBottoniGO.clear();
	coloreBottoniGO.clear();
	updateText2(&textGameOver, { 1.0, 1.0, 1.0, 0.0 }, textGameOver.scale, stringToCharBuff("GAME OVER"));
	disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.0, 0.8, 0.0, 0.0 }, width * 240 / 1000, width * 450 / 1000, height * 150 / 1000, height * 280 / 1000);
	disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.8, 0.0, 0.0, 0.0 }, width * 760 / 1000, width * 550 / 1000, height * 150 / 1000, height * 280 / 1000);
	coloreContornoBGO.clear();
	for (int i = 0; i < 16; i++)
		coloreContornoBGO.push_back({ 1.0, 1.0, 1.0, 0.0 });
	disegnaPerks.clear();
	colorePerks.clear();
	disegnaPerksT.clear();
	colorePerks.clear();

	disegnaBackgroundMenu.clear();
	coloreBackgroundMenu.clear();
	disegnaContornoMenu.clear();
	coloreContornoMenu.clear();
	disegnaBox.clear();
	coloreBox.clear();
	disegnaLucchetto.clear();
	coloreArcoLucchetto.clear();
	disegnaArcoLucchetto.clear();
	coloreArcoLucchetto.clear();
	menuGraficheInit();
	stageFinal.clear();
	initStages();

	updateText(&textStatsBar.at(1), intToCharBuff(game.lives));
	updateText(&textStatsBar.at(3), intToCharBuff(game.bombs));
	updateText(&textStatsBar.at(5), intToCharBuff(game.points));
	updateText(&textStatsBar.at(8), intToCharBuff(game.score));

	updateText(&textMenu.at(1), intToCharBuff(game.points));
	updateText(&textMenu.at(5), intToCharBuff(game.fp + 1));
	updateText(&textMenu.at(7), intToCharBuff(game.h + 1));
	updateText(&textMenu.at(9), intToCharBuff(game.s + 1));
	updateText(&textMenu.at(14), intToCharBuff(game.bombs));
	updateText(&textMenu.at(16), intToCharBuff(game.lives));

	postmortem3 = false;
	postmortem2 = true;
	postmortem_c = INVULNERABILITY * 2 / 5;
	disegnaNav = true;
}

void checkStage() // cambio stage se nemici_per_stage locale raggiunto
{
	// PARTE NUOVA
	if (game.currentStage != game.totalStages)
	{
		Wave w = stageFinal.at(game.currentStage).waves.at(game.currentWave);

		new_timestamp = glutGet(GLUT_ELAPSED_TIME);
		int delta1 = oldTimeSinceStart - game.waveInitTimestamp;
		int delta2 = new_timestamp - game.waveInitTimestamp;
		bool atLeastOnce = false;

		for (int i = 0; i < w.groups.size(); i++) // potrebbero esserci più gruppi che hanno un timestamp
		{										  // contenuto tra old_t e new_t: ciclo sulla wave intera
			Group g = w.groups.at(i);
			if (delta1 < (g.time * 1000 + deltaPause) && delta2 >= (g.time * 1000 + deltaPause) &&
				!g.spawned)
			{
				stageFinal.at(game.currentStage).waves.at(game.currentWave).groups.at(i).spawned = true;
				initNemici(w.groups.at(i).num, game.currentWave, w.groups.at(i).type);
				std::cout << "spawno gruppo: " << i << "\n";
			}
		}

		oldTimeSinceStart = new_timestamp;

		// check fine wave
		if (game.waveKills == stageFinal.at(game.currentStage).waves.at(game.currentWave).totEnemies) // se == al totale dell'ondata: ondata finita, c_waves++
		{	// check fine stage
			if (game.currentStage == 0 && game.currentWave == 0)
			{
				game.unlockedPerks++;
				unlockPerk();
			}
			if (game.stageKills == stageFinal.at(game.currentStage).totEnemies) // totale nemici stage
			{
				game.currentStage++;
				game.unlockedPerks++;
				unlockPerk();
				if (game.currentStage == 2 || game.currentStage == 5)
					game.maxPerks++;
				game.stageKills = 0;
				game.currentWave = 0;
			}
			else
			{
				game.currentWave++;
			}
			game.waveKills = 0;
			game.currentGroup = 0;
			deltaPause = 0;
			game.waveInitTimestamp = glutGet(GLUT_ELAPSED_TIME);
			oldTimeSinceStart = game.waveInitTimestamp - 1;
		}

	}
	// check fine game
	if (game.currentStage == game.totalStages)
	{
		endOfGame();
	}
}

void update(int a)
{
	if (mainMenu)
	{

	}
	else
	{
		if (gameOver) // hardcode indecente
		{
			if (!waitingBounties && !postmortem3)
			{
				bool trovato = false;
				for (int i = 0; i < proiettili.size() && !trovato; i++)
				{
					if (proiettili.at(i).type == 1)
						trovato = true;
				}
				if (!trovato)
				{
					waitingBounties = true;
					textPunteggio.clear();
					int finalScore = game.score + game.points;
					snprintf(buffer, 32, "TOTAL SCORE: %5d", finalScore);
					textPunteggio.push_back(createText2(width * 280 / 1000, height * 425 / 1000, false, 10.0, true, buffer, { 1.0, 1.0, 1.0, 0.0 }));
					textPunteggio.push_back(createText2(width * 280 / 1000, height * 215 / 1000, false, 6.0, true, "PLAY AGAIN", { 1.0, 1.0, 1.0, 0.0 }));
					textPunteggio.push_back(createText2(width * 600 / 1000, height * 215 / 1000, false, 6.0, true, "MAIN MENU", { 1.0, 1.0, 1.0, 0.0 }));

					// codice necessario solo se esiste un bottone rapido per testare le scritte di Game Over
					scalaGameOver = 100.0;
					fattoreRiduzione = 0.01;
					progressiveTranspGO = 0.0;
					progressiveTranspPunteggio = 0.01;
					textGameOver = createText2(width * 78 / 1000, height * 638 / 1000, true, scalaGameOver, true, "GAME OVER", { 1.0, 1.0, 1.0, 0.0 });
				}
			}
			else
			{
				if (textGameOver.scale > 30)
				{
					//std::cout << "\ntextGameOver.scale: " << textGameOver.scale << ";    fattoreRiduzione: " << fattoreRiduzione << ";";
					textGameOver.pos.x += 15; //caso preciso e centrato: pos.x+=11   e fattore riduz*=100
					scalaGameOver -= fattoreRiduzione * 100;
					if (progressiveTranspGO < 1.0 && textGameOver.scale < 98)
						progressiveTranspGO += 0.04;
					fattoreRiduzione *= 1.13;
					textGameOver.scale -= fattoreRiduzione;
					updateText2(&textGameOver, { 1.0, 1.0, 1.0, progressiveTranspGO }, textGameOver.scale, stringToCharBuff("GAME OVER"));
				}
				else
				{
					if (progressiveTranspPunteggio < 1.0)
					{
						progressiveTranspPunteggio += 0.05;
						updateText2(&textPunteggio.at(0), { 1.0, 1.0, 1.0, progressiveTranspPunteggio }, textPunteggio.at(0).scale, buffer);
						updateText2(&textPunteggio.at(1), { 1.0, 1.0, 1.0, progressiveTranspPunteggio }, textPunteggio.at(1).scale, stringToCharBuff("PLAY AGAIN"));
						updateText2(&textPunteggio.at(2), { 1.0, 1.0, 1.0, progressiveTranspPunteggio }, textPunteggio.at(2).scale, stringToCharBuff("MAIN MENU"));
						disegnaBottoniGO.clear();
						coloreBottoniGO.clear();
						disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.0, 0.8, 0.0, progressiveTranspPunteggio / 2 }, width * 240 / 1000, width * 450 / 1000, height * 150 / 1000, height * 280 / 1000);
						disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.8, 0.0, 0.0, progressiveTranspPunteggio / 2 }, width * 760 / 1000, width * 550 / 1000, height * 150 / 1000, height * 280 / 1000);
						coloreContornoBGO.clear();
						for (int i = 0; i < 16; i++)
							coloreContornoBGO.push_back({ 1.0, 1.0, 1.0, progressiveTranspPunteggio });
					}
					else
						stopFlow = true;
				}
			}
		}
		if (!pause)
		{
			if (recharging)
			{
				int delta = glutGet(GLUT_ELAPSED_TIME) - cooldownStart;
				if (delta > (COOLDOWN * game.cdmultiplier))
					recharging = false;
			}
			if (mouseLeft_down && !gameOver)
				fuoco();

			if (sequenza_bomba > 0)
			{
				disegnaBomba.clear();
				coloreBomba.clear();
				disegnaCirconferenza(&disegnaBomba, &coloreBomba, { 1.0, 1.0, 1.0, 1.0 }, bombaPos, windowDiagonal * sequenza_bomba / numFramesBomba, 120, 0, 0, false);
				sequenza_bomba++;
				if (sequenza_bomba == numFramesBomba)
					sequenza_bomba = 0;
			}

			// check collisioni
			if (!postmortem1 && !postmortem3)
			{
				checkCollisioni();
				int i_max = nemici.size();
				for (int i = 0; i < i_max; i++)
				{
					if (nemici.at(i).flag == true)
					{
						distruggiNemico(i);
						i_max--;
						i--;
					}
				}
				i_max = proiettili.size();
				for (int i = 0; i < i_max; i++)
				{
					if (proiettili.at(i).type == 0 && proiettili.at(i).flag == 1)
					{
						proiettili.erase(proiettili.begin() + i);
						i_max--;
						i--;
					}
					// ? v
					else if (proiettili.at(i).type == 2 && proiettili.at(i).flag == 1)
					{
						bool trovato = false;
						for (int j = 0; j < proiettili_verdi.size() && !trovato; j++)
							if (proiettili_verdi.at(j) == proiettili.at(i).id)
							{
								trovato = true;
								proiettili_verdi.erase(proiettili_verdi.begin() + j);
							}
						proiettili.erase(proiettili.begin() + i);
						i_max--;
						i--;
					}
				}
			}

			if (!gameOver && !stopFlow)
				checkStage();

			updateNemici();
			updateScie();
			updateResidui();
			updateProiettili();
			int i_max = proiettili.size();
			for (int i = 0; i < i_max; i++)
			{
				Proiettile p = proiettili.at(i);
				if (p.type == 0 && p.flag > 0)
				{
					update_dodged(p.id);
					proiettili.erase(proiettili.begin() + i);
					i_max--;
					i--;
				}
			}

			if (game.health < game.maxHealth && game.health > 0)
			{
				game.health += 0.03;

				updateHealthBar(game.health);
			}

			if (postmortem1)
			{
				disegnaNav = false;
				postmortem_c++;
				if (postmortem_c == INVULNERABILITY * 2 / 5)
				{
					postmortem1 = false;
					postmortem2 = true;
					player.pos = { width / 2, height / 2 };
					disegnaNav = true;
				}
			}
			if (postmortem2)
			{
				game.health = game.maxHealth;
				updateHealthBar(game.health);
				postmortem_c++;
				if ((postmortem_c % intermittenza) == 0)
					disegnaNav = !disegnaNav;
				if (postmortem_c == INVULNERABILITY)
				{
					postmortem2 = false;
					disegnaNav = true;
					postmortem_c = 0;
				}
			}

			//aggiornamento velocità
			if (pressing_down)
			{
				if (player.vel4.dw > -max_vel)
					player.vel4.dw = player.vel4.dw - vel_inc;
				if (player.vel4.dw < -max_vel)
					player.vel4.dw = -max_vel;
				//player.pos.y -= 10.0;
			}
			else {
				if (player.vel4.dw < 0.0)
					player.vel4.dw = player.vel4.dw + vel_dec;
				if (player.vel4.dw > 0.0)
					player.vel4.dw = 0.0;
			}
			if (pressing_up)
			{
				if (player.vel4.up < max_vel)
					player.vel4.up = player.vel4.up + vel_inc;
				if (player.vel4.up > max_vel)
					player.vel4.up = max_vel;
				//player.pos.y += 10.0;
			}
			else {
				if (player.vel4.up > 0.0)
					player.vel4.up = player.vel4.up - vel_dec;
				if (player.vel4.up < 0.0)
					player.vel4.up = 0.0;
			}
			if (pressing_left)
			{
				if (player.vel4.sx > -max_vel)
					player.vel4.sx = player.vel4.sx - vel_inc;
				if (player.vel4.sx < -max_vel)
					player.vel4.sx = -max_vel;
				//player.pos.x -= 10.0;
			}
			else {
				if (player.vel4.sx < 0.0)
					player.vel4.sx = player.vel4.sx + vel_dec;
				if (player.vel4.sx > 0.0)
					player.vel4.sx = 0.0;
			}
			if (pressing_right)
			{
				if (player.vel4.dx < max_vel)
					player.vel4.dx = player.vel4.dx + vel_inc;
				if (player.vel4.dx > max_vel)
					player.vel4.dx = max_vel;
				//player.pos.x += 10.0;
			}
			else
			{
				if (player.vel4.dx > 0.0)
					player.vel4.dx = player.vel4.dx - vel_dec;
				if (player.vel4.dx < 0.0)
					player.vel4.dx = 0.0;
			}

			//aggiornamento posizione
			player.pos.x = player.pos.x + player.vel4.sx + player.vel4.dx;
			player.pos.y = player.pos.y + player.vel4.up + player.vel4.dw;

			if (player.pos.x < padding.x)
			{
				player.pos.x = padding.x;
				player.vel4.dx = player.vel4.dx - (player.vel4.sx * 0.8);
				player.vel4.sx = 0.0f;
			}
			if (player.pos.x > width - padding.x)
			{
				player.pos.x = width - padding.x;
				player.vel4.sx = player.vel4.sx - (player.vel4.dx * 0.8);
				player.vel4.dx = 0.0f;
			}
			if (player.pos.y < padding.y)
			{
				player.pos.y = padding.y;
				player.vel4.up = player.vel4.up - (player.vel4.dw * 0.8);
				player.vel4.dw = 0.0f;
			}
			if (player.pos.y > height - padding.y)
			{
				player.pos.y = height - padding.y;
				player.vel4.dw = player.vel4.dw - (player.vel4.up * 0.8);
				player.vel4.up = 0.0f;
			}
			if (!stopFlow)
				player.angle = getAngle(player.pos, mouseInput) * 0.0174533;

		}
		c++;
	}
	glutTimerFunc(1, update, 0);

}

void updateScie()
{
	std::vector<vec2> temp;
	int size;
	float radius;
	float bigRadius;
	float lowRadius;

	//player
	if (!postmortem1 && !postmortem3)
	{
		temp = player.scia;
		player.scia.clear();
		size = (temp.size() < LEN_SCIA) ? temp.size() : (LEN_SCIA - 1);
		player.scia.push_back(player.pos);
		for (int i = 0; i < size; i++)
		{
			player.scia.push_back(temp.at(i));
		}
		//draw player scia
		player.puntiScia.clear();
		player.coloreScia.clear();
		player.puntiScia2.clear();
		player.coloreScia2.clear();

		radius = nav_raggio * player.scale;

		disegnaCoronaCircolare(&player.puntiScia, &player.coloreScia, player.scia.at(0), { 1.0, 1.0, 1.0, 0.5f }, { 1.0, 1.0, 1.0, 0.0f }, radius, radius + 6, 30, PI * 2, 0.0);
		disegnaCoronaCircolare(&player.puntiScia, &player.coloreScia, player.scia.at(0), { 1.0, 1.0, 1.0, 0.0f }, { 1.0, 1.0, 1.0, 0.5f }, radius - 6, radius, 30, PI * 2, 0.0);

		bigRadius = radius + 10;
		lowRadius = radius - 3;
		if (player.scia.size() > 0)
		{
			disegnaScia(player.scia, &player.puntiScia, &player.coloreScia, { 1.0, 1.0, 1.0, 0.5f }, bigRadius, lowRadius, 60, player.vel4);
			disegnaScia(player.scia, &player.puntiScia, &player.coloreScia, { 1.0, 1.0, 1.0, 0.2f }, bigRadius - 3, lowRadius - 3, 60, player.vel4);
			if (showTrails)
				disegnaLineeScia(player.scia, &player.puntiScia2, &player.coloreScia2, { 1.0, 0.0, 0.0, 1.0 }, bigRadius, lowRadius, 10, player.vel4);

		}
	}

	//nemici
	for (int i = 0; i < nemici.size(); i++)
	{
		temp = nemici.at(i).nav.scia; //vettore temp per popolare la scia progressivamente
		nemici.at(i).nav.scia.clear();
		size = (temp.size() < lunghezzeScia[nemici.at(i).type] ? temp.size() : lunghezzeScia[nemici.at(i).type] - 1);
		//if (!frameSkipperBlueN)
		nemici.at(i).nav.scia.push_back(nemici.at(i).nav.pos);
		//frameSkipperBlueN = !frameSkipperBlueN;
		for (int j = 0; j < size; j++)
		{
			nemici.at(i).nav.scia.push_back(temp.at(j));
		}

		//draw nemici scia
		nemici.at(i).nav.puntiScia.clear();
		nemici.at(i).nav.coloreScia.clear();
		nemici.at(i).nav.puntiScia2.clear();
		nemici.at(i).nav.coloreScia2.clear();

		Nemico n = nemici.at(i);
		if (n.type == 2) // disegno particolare nemici di tipo 2 (doppio colore
		{
			for (int k = 1; k < n.nav.scia.size() / 2; k++)
				disegnaCoronaCircolare(&nemici.at(i).nav.puntiScia, &nemici.at(i).nav.coloreScia, nemici.at(i).nav.scia.at(k * 2),
					{ 1.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, raggi[n.type] * scala / 2 + 1, raggi[n.type] * scala / 2 - 3, 30, PI * 2, 0.0);
		}

		radius = raggi[nemici.at(i).type] * scala;
		vec4 n_colore = n_colori[nemici.at(i).type];
		n_colore.a = 0.0;
		vec4 n_colore2 = n_colore;
		n_colore2.a = 0.5;
		vec4 n_colore3 = n_colore;
		n_colore3.a = 0.2;

		disegnaCoronaCircolare(&nemici.at(i).nav.puntiScia, &nemici.at(i).nav.coloreScia, nemici.at(i).nav.scia.at(0), n_colore2, n_colore, radius, radius + 6, 30, PI * 2, 0.0);
		disegnaCoronaCircolare(&nemici.at(i).nav.puntiScia, &nemici.at(i).nav.coloreScia, nemici.at(i).nav.scia.at(0), n_colore, n_colore2, radius - 6, radius, 30, PI * 2, 0.0);

		bigRadius = radius + 10;
		lowRadius = radius - 3;
		if (nemici.at(i).nav.scia.size() > 0)
		{
			vec2 vel = nemici.at(i).nav.vel;
			Vel4 v = { vel.x, vel.y, vel.x, vel.y };
			disegnaScia(nemici.at(i).nav.scia, &nemici.at(i).nav.puntiScia, &nemici.at(i).nav.coloreScia, n_colore2, bigRadius, lowRadius, 60, v);
			disegnaScia(nemici.at(i).nav.scia, &nemici.at(i).nav.puntiScia, &nemici.at(i).nav.coloreScia, n_colore3, bigRadius - 3, lowRadius - 3, 60, v);
			if (showTrails)
				disegnaLineeScia(nemici.at(i).nav.scia, &nemici.at(i).nav.puntiScia2, &nemici.at(i).nav.coloreScia2, { 1.0, 0.0, 0.0, 1.0 }, bigRadius, lowRadius, 10, v);
		}
	}
	temp.clear();
}

void onWindowResize(int w, int h)
{
	glutReshapeWindow(1280, 720);
	/*
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	*/
}

void sparaProiettile(int i, float x, float y)
{
	float compX;
	float compY;
	float angleStart = player.angle - game.fp * FP_ANGLE / 2;
	if (i == -2) // proiettile nemico tipo 5
	{
		Proiettile p = {};
		p.type = 2;
		p.pos = { x, y };
		p.angle = getAngle({ x, y }, player.pos) * 0.0174533;
		p.pastPos = p.pos;
		p.id = counter_proiettili;
		counter_proiettili++;
		p.flag = 0;
		p.bounces = 0;
		compX = cos(p.angle);
		compY = sin(p.angle);
		p.pos2 = { compX * lunghezzaProiettile + p.pos.x, compY * lunghezzaProiettile + p.pos.y };
		p.vel = { compX * velocitaProiettili * 1 / 5, compY * velocitaProiettili * 1 / 5 };
		proiettili.push_back(p);
		int xA = 0;
	}
	else if (i == -1) // proiettile
	{
		for (int j = 0; j < game.fp + 1; j++)
		{
			Proiettile p = {};
			p.type = 0;
			p.pos = player.pos;
			p.angle = angleStart + j * FP_ANGLE;
			p.pastPos = p.pos;
			p.flag = 0;
			if (game.perksActive[1])
				p.bounces = 2;
			else
				p.bounces = 0;
			if (game.perksActive[2])
				p.health = 2;
			else
				p.health = 0;
			p.id = counter_proiettili;
			counter_proiettili++;
			compX = cos(p.angle);
			compY = sin(p.angle);
			p.pos2 = { compX * lunghezzaProiettile + p.pos.x, compY * lunghezzaProiettile + p.pos.y };
			p.vel = { compX * velocitaProiettili, compY * velocitaProiettili };
			proiettili.push_back(p);
		}
	}
	else // ricompensa
	{
		Proiettile p = {};
		p.type = 1;
		float new_angle = fmodf(getRandomFloat(player.angle + PI / 2, player.angle + PI + PI / 2), 2 * PI);
		p.pos = { x, y };
		compX = cos(new_angle);
		compY = sin(new_angle);
		p.pos2 = { compX * lunghezzaRicompense + p.pos.x, compY * lunghezzaRicompense + p.pos.y };
		p.vel = { compX * velocitaRicompense, compY * velocitaRicompense };
		p.angle = new_angle;
		float angleDiff = p.angle - (getAngle(p.pos, player.pos) * 0.0174533);

		proiettili.push_back(p);
	}
}

void updateProiettili()
{
	disegnaProiettili.clear();
	coloriProiettili.clear();

	for (int i = 0; i < proiettili.size(); i++)
	{
		if (proiettili.at(i).type == 0 || proiettili.at(i).type == 2) //proiettili del giocatore o di nemici di tipo 5
		{

			Proiettile p = proiettili.at(i);
			//controllo quali proiettili sono usciti dalla finestra e vanno rimossi
			if (p.pos.x > width || p.pos.x < 0 || p.pos.y > height || p.pos.y < 0)
			{
				if (p.pos.x > width)
					if (p.bounces > 0)
					{
						proiettili.at(i).bounces--;
						proiettili.at(i).vel.x = -p.vel.x;
						proiettili.at(i).pastPos = proiettili.at(i).pos;
						proiettili.at(i).pos.x += proiettili.at(i).vel.x;
						proiettili.at(i).pos.y += proiettili.at(i).vel.y;
						float compX = proiettili.at(i).vel.x / velocitaProiettili;
						float compY = proiettili.at(i).vel.y / velocitaProiettili;
						proiettili.at(i).pos2 = { compX * lunghezzaProiettile + proiettili.at(i).pos.x + proiettili.at(i).vel.x,
							compY * lunghezzaProiettile + proiettili.at(i).pos.y + proiettili.at(i).vel.y };
					}
					else
						proiettili.at(i).flag++;
				if (p.pos.x < 0)
					if (p.bounces > 0)
					{
						proiettili.at(i).bounces--;
						proiettili.at(i).vel.x = -p.vel.x;
						proiettili.at(i).pastPos = proiettili.at(i).pos;
						proiettili.at(i).pos.x += proiettili.at(i).vel.x;
						proiettili.at(i).pos.y += proiettili.at(i).vel.y;
						float compX = proiettili.at(i).vel.x / velocitaProiettili;
						float compY = proiettili.at(i).vel.y / velocitaProiettili;
						proiettili.at(i).pos2 = { compX * lunghezzaProiettile + proiettili.at(i).pos.x + proiettili.at(i).vel.x,
							compY * lunghezzaProiettile + proiettili.at(i).pos.y + proiettili.at(i).vel.y };
					}
					else
						proiettili.at(i).flag++;
				if (p.pos.y > height)
					if (p.bounces > 0)
					{
						proiettili.at(i).bounces--;
						proiettili.at(i).vel.y = -p.vel.y;
						proiettili.at(i).pastPos = proiettili.at(i).pos;
						proiettili.at(i).pos.x += proiettili.at(i).vel.x;
						proiettili.at(i).pos.y += proiettili.at(i).vel.y;
						float compX = proiettili.at(i).vel.x / velocitaProiettili;
						float compY = proiettili.at(i).vel.y / velocitaProiettili;
						proiettili.at(i).pos2 = { compX * lunghezzaProiettile + proiettili.at(i).pos.x + proiettili.at(i).vel.x,
							compY * lunghezzaProiettile + proiettili.at(i).pos.y + proiettili.at(i).vel.y };
					}
					else
						proiettili.at(i).flag++;
				if (p.pos.y < 0)
					if (p.bounces > 0)
					{
						proiettili.at(i).bounces--;
						proiettili.at(i).vel.y = -p.vel.y;
						proiettili.at(i).pastPos = proiettili.at(i).pos;
						proiettili.at(i).pos.x += proiettili.at(i).vel.x;
						proiettili.at(i).pos.y += proiettili.at(i).vel.y;
						float compX = proiettili.at(i).vel.x / velocitaProiettili;
						float compY = proiettili.at(i).vel.y / velocitaProiettili;
						proiettili.at(i).pos2 = { compX * lunghezzaProiettile + proiettili.at(i).pos.x + proiettili.at(i).vel.x,
							compY * lunghezzaProiettile + proiettili.at(i).pos.y + proiettili.at(i).vel.y };
					}
					else
						proiettili.at(i).flag++;
			}
			else // niente rimbalzo
			{
				proiettili.at(i).pastPos = proiettili.at(i).pos;
				proiettili.at(i).pos2.x += proiettili.at(i).vel.x;
				proiettili.at(i).pos.x += proiettili.at(i).vel.x;
				proiettili.at(i).pos2.y += proiettili.at(i).vel.y;
				proiettili.at(i).pos.y += proiettili.at(i).vel.y;
			}
		}
		else // comportamento ricompense
		{
			proiettili.at(i).pos2.x = proiettili.at(i).pos.x;
			proiettili.at(i).pos.x += proiettili.at(i).vel.x;
			proiettili.at(i).pos2.y = proiettili.at(i).pos.y;
			proiettili.at(i).pos.y += proiettili.at(i).vel.y;

			if (postmortem1 || postmortem3) { // CODICE RIMBALZO RICOMPENSE SU PARETI: SOLO DURANTE SEQUENZA PLAYER MORTO
				if (proiettili.at(i).pos.x < 0)
				{
					proiettili.at(i).pos.x = 0;
					proiettili.at(i).vel.x = -proiettili.at(i).vel.x;
				}
				if (proiettili.at(i).pos.x > width - 0)
				{
					proiettili.at(i).pos.x = width - 0;
					proiettili.at(i).vel.x = -proiettili.at(i).vel.x;
				}
				if (proiettili.at(i).pos.y < 0)
				{
					proiettili.at(i).pos.y = 0;
					proiettili.at(i).vel.y = -proiettili.at(i).vel.y;
				}
				if (proiettili.at(i).pos.y > height - 0)
				{
					proiettili.at(i).pos.y = height - 0;
					proiettili.at(i).vel.y = -proiettili.at(i).vel.y;
				}
			}
			else
			{
				Proiettile p = proiettili.at(i);

				float angle1 = p.angle;
				float angle2 = getAngle(p.pos, player.pos) * 0.0174533;

				float angleDiff = angle1 - angle2;

				if (angleDiff < 0)	// check affinché la differenza sia sempre positiva (fini di calcolo)
					angleDiff = -angleDiff;


				//std::cout << "angolo1: " << angle1 << "  ---  ";
				//std::cout << "angoloCONplayer: " << angle2 << "  ---  ";
				//std::cout << "anglediff: " << angleDiff << "\n";

				if ((angle1 < angle2) && (angleDiff <= PI) || (angle1 > angle2) && (angleDiff > PI))
				{
					if ((p.angle + scartoAngolare) > (2 * PI))
						p.angle = p.angle + scartoAngolare - (2 * PI);
					else
						p.angle += scartoAngolare;

					float compX = cos(p.angle);
					float compY = sin(p.angle);
					proiettili.at(i).vel = { compX * velocitaRicompense, compY * velocitaRicompense };
					proiettili.at(i).angle = p.angle;
				}
				else
				{
					if ((p.angle - scartoAngolare) < 0)
						p.angle = (2 * PI) + p.angle - scartoAngolare;
					else
						p.angle -= scartoAngolare;

					float compX = cos(p.angle);
					float compY = sin(p.angle);
					proiettili.at(i).vel = { compX * velocitaRicompense, compY * velocitaRicompense };
					proiettili.at(i).angle = p.angle;
				}
			}
		}

	}

	// popolamento array dei vertici per disegnare i proiettili
	for (int i = 0; i < proiettili.size(); i++)
	{
		disegnaProiettili.push_back(proiettili.at(i).pos);
		disegnaProiettili.push_back(proiettili.at(i).pos2);
		if (proiettili.at(i).type == 0)
		{
			coloriProiettili.push_back({ 1.0f, 0.0f, 0.0f, 1.0f });
			coloriProiettili.push_back({ 1.0f, 0.0f, 0.0f, 1.0f });
		}
		else if (proiettili.at(i).type == 1)
		{
			coloriProiettili.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
			coloriProiettili.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });

		}
		else if (proiettili.at(i).type == 2)
		{
			coloriProiettili.push_back({ 0.0f, 1.0f, 0.0f, 1.0f });
			coloriProiettili.push_back({ 0.0f, 1.0f, 0.0f, 1.0f });

		}
	}
}

bool isInHalfPlane(vec2 a, vec2 b, vec2 c) // a,b punti retta; true = c appartiene al semipiano superiore
{
	return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) < 0;
}

bool isInRegion(Proiettile p, Nemico n)
{
	vec2 p0 = p.pos;
	float compX = cos(p.angle + PI / 4);
	float compY = sin(p.angle + PI / 4);
	vec2 p1 = { p.pos.x + compX * 100, p.pos.y + compY * 100 };
	compX = cos(p.angle - PI / 4);
	compY = sin(p.angle - PI / 4);
	vec2 p2 = { p.pos.x + compX * 100, p.pos.y + compY * 100 };
	vec2 p3 = n.nav.pos;
	compX = cos(p.angle + PI / 2);
	compY = sin(p.angle + PI / 2);
	vec2 p4 = { p.pos.x + compX * 100, p.pos.y + compY * 100 };
	compX = cos(p.angle - PI / 2);
	compY = sin(p.angle - PI / 2);
	vec2 p5 = { p.pos.x + compX * 100, p.pos.y + compY * 100 };
	return isInHalfPlane(p0, p1, p3) && !isInHalfPlane(p0, p2, p3);
}

void updateNemici()
{
	for (int i = 0; i < nemici.size(); i++)
	{
		int t = nemici.at(i).type;
		if (t == 1 || t == 2 || t == 3)
		{
			if (!postmortem1 && !postmortem3)
			{
				float angle = getAngle(nemici.at(i).nav.pos, player.pos) * 0.0174533;
				nemici.at(i).nav.angle = angle;
				nemici.at(i).nav.vel = { cos(angle) * velocita[t], sin(angle) * velocita[t] };
			}
		}
		if (t == 4)
		{
			if (!postmortem1 && !postmortem3)
			{
				Nemico n = nemici.at(i);
				if (n.dodging == 0)
				{
					float min_distance = 30;
					int id_proiettile;
					for (int j = 0; j < proiettili.size(); j++) //cerco il proiettile la cui traiettoria ha la distanza minore dal nemico
					{
						if (proiettili.at(j).type == 0)
						{
							Proiettile p = proiettili.at(j);
							vec2 a = p.pos;
							vec2 b = { p.pos.x + cos(p.angle + PI / 2) * 100, p.pos.y + sin(p.angle + PI / 2) * 100 };
							vec2 c = n.nav.pos;
							if (isInHalfPlane(a, b, c))
							{
								std::cout << "\nPARTE GIUSTA DEL PIANO";
								float distance = getDistancePointToLine(n.nav.pos, p.pos, p.pos2);
								if (distance < min_distance && !search_dodged(i, j)) //controllo solo i proiettili non già dodgiati
								{
									min_distance = distance;	//se ci sono proiettili più vicini della distanza entro cui il dodge si
									id_proiettile = j;			//attiva, voglio dare priorità a quello con distanza minore dal nemico
								}
							}
							// calcolare anche la distanza tra proiettile e nemico complessiva, per tenerla in conto nel calcolo dell'angolo
							// della direzione in cui avverrà in dodge: più il nemico è vicino e più il dodge sarà verso l'indietro; più
							// il nemico è lontano e più può permettersi di dodgiare solo lateralmente
						}
					}
					if (min_distance < 30)
					{
						std::cout << "\nDODGING: " << min_distance;
						nemici.at(i).dodging = DODGING;
						float angle_p = proiettili.at(id_proiettile).angle;
						float distance_p_n = getDistance(n.nav.pos, proiettili.at(id_proiettile).pos);
						if (isInHalfPlane(proiettili.at(id_proiettile).pos, proiettili.at(id_proiettile).pos2, n.nav.nextPos))	//calcolare angolo ( (-)PI/2 se lontano(lateralmente), (-)PI*5/6 se vicino)
							nemici.at(i).nav.angle = angle_p - PI / 4;
						else
							nemici.at(i).nav.angle = angle_p + PI / 4;
						nemici.at(i).nav.vel = { cos(nemici.at(i).nav.angle) * velocita[t], sin(nemici.at(i).nav.angle) * velocita[t] };
					}
					else
					{
						if (!postmortem1 && !postmortem3)
						{
							float angle = getAngle(nemici.at(i).nav.pos, player.pos) * 0.0174533;
							nemici.at(i).nav.angle = angle;
							nemici.at(i).nav.vel = { cos(angle) * velocita[t], sin(angle) * velocita[t] };
							nemici.at(i).nav.nextPos = { nemici.at(i).nav.pos.x + nemici.at(i).nav.vel.x,
															nemici.at(i).nav.pos.y + nemici.at(i).nav.vel.y };
						}
					}
				}
				else
				{
					nemici.at(i).dodging--;
					nemici.at(i).nav.nextPos = { nemici.at(i).nav.pos.x + nemici.at(i).nav.vel.x,
													nemici.at(i).nav.pos.y + nemici.at(i).nav.vel.y };
				}
			}
		}
		if (t == 5)
		{
			if (!postmortem1 && !postmortem3)
			{
				if (nemici.at(i).cooldown == 0) // il nemico di tipo 5 spara un proiettile
				{
					sparaProiettile(-2, nemici.at(i).nav.pos.x, nemici.at(i).nav.pos.y);
					nemici.at(i).cooldown = COOLDOWN5;
				}
				else
					nemici.at(i).cooldown--;
				if (getDistance(nemici.at(i).nav.pos, player.pos) < SOGLIA5)
				{
					nemici.at(i).revolving = true;
				}
				else if (getDistance(nemici.at(i).nav.pos, player.pos) > SOGLIA5 + 10)
					nemici.at(i).revolving = false;

				float angle = getAngle(nemici.at(i).nav.pos, player.pos) * 0.0174533;
				if (nemici.at(i).revolving)
					angle = angle + PI / 2 - 0.01;
				nemici.at(i).nav.angle = angle;
				nemici.at(i).nav.vel = { cos(angle) * velocita[t], sin(angle) * velocita[t] };
			}
		}
		if (t == 6)
		{
			if (nemici.at(i).cooldown == 0) // il nemico di tipo 5 spara un proiettile
			{
				if (nemici.size() < MAXNEMICI)
				{
					Nemico n = {};
					vec2 pos = nemici.at(i).nav.pos;
					vec2 vel = { -nemici.at(i).nav.vel.x, -nemici.at(i).nav.vel.y };
					n.id = nemici.at(i).id;
					n.flag = false;
					n.type = 6;
					n.nav.pos = pos;
					n.nav.vel = vel;
					n.nav.nextPos = { n.nav.pos.x + n.nav.vel.x, n.nav.pos.y + n.nav.vel.y };
					n.cooldown = COOLDOWN6;
					n.child = true;
					nemici.push_back(n);

					// DISEGNO NEMICI
					//genero VAO nemici
					glGenVertexArrays(1, &nemici.at(nemici.size() - 1).nav.VAO);
					glBindVertexArray(nemici.at(nemici.size() - 1).nav.VAO);
					//vertici
					glGenBuffers(1, &nemici.at(nemici.size() - 1).nav.VBO_Geom);
					glBindBuffer(GL_ARRAY_BUFFER, nemici.at(nemici.size() - 1).nav.VBO_Geom);
					//colori
					glGenBuffers(1, &nemici.at(nemici.size() - 1).nav.VBO_Col);
					glBindBuffer(GL_ARRAY_BUFFER, nemici.at(nemici.size() - 1).nav.VBO_Col);

					nemici.at(nemici.size() - 1).nav.scale = scala;
					disegnaNemico(nemici.size() - 1); // qui inserisco quelli di ciascun nemico
					glBindBuffer(GL_ARRAY_BUFFER, nemici.at(nemici.size() - 1).nav.VBO_Geom);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, nemici.at(nemici.size() - 1).nav.VBO_Col);
					glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(1);
					glBindVertexArray(0);


					glGenVertexArrays(1, &nemici.at(nemici.size() - 1).nav.VAO_S);
					glBindVertexArray(nemici.at(nemici.size() - 1).nav.VAO_S);
					//vertici
					glGenBuffers(1, &nemici.at(nemici.size() - 1).nav.VBO_S);
					glBindBuffer(GL_ARRAY_BUFFER, nemici.at(nemici.size() - 1).nav.VBO_S);
					//colori
					glGenBuffers(1, &nemici.at(nemici.size() - 1).nav.VBO_SC);
					glBindBuffer(GL_ARRAY_BUFFER, nemici.at(nemici.size() - 1).nav.VBO_SC);
				}
				nemici.at(i).cooldown = COOLDOWN6;
			}
			else
				nemici.at(i).cooldown--;
		}
		if (nemici.at(i).nav.pos.x < padding.x)
		{
			nemici.at(i).nav.pos.x = padding.x;
			nemici.at(i).nav.vel.x = -nemici.at(i).nav.vel.x;
		}
		if (nemici.at(i).nav.pos.x > width - padding.x)
		{
			nemici.at(i).nav.pos.x = width - padding.x;
			nemici.at(i).nav.vel.x = -nemici.at(i).nav.vel.x;
		}
		if (nemici.at(i).nav.pos.y < padding.y)
		{
			nemici.at(i).nav.pos.y = padding.y;
			nemici.at(i).nav.vel.y = -nemici.at(i).nav.vel.y;
		}
		if (nemici.at(i).nav.pos.y > height - padding.y)
		{
			nemici.at(i).nav.pos.y = height - padding.y;
			nemici.at(i).nav.vel.y = -nemici.at(i).nav.vel.y;
		}
		nemici.at(i).nav.pos.x += nemici.at(i).nav.vel.x;
		nemici.at(i).nav.pos.y += nemici.at(i).nav.vel.y;

	}
}

void fuoco()
{
	if (!recharging)
	{
		playSoundEffect(-1, "fire");
		recharging = true;
		cooldownStart = glutGet(GLUT_ELAPSED_TIME);
		sparaProiettile(-1, 0.0, 0.0);
	}
}

void mouseClickEvent(int button, int state, int x, int y)
{
	if (mainMenu)
	{
		if (hoverMM[0])
			if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			{
				hoverMM[0] = false;
				mainMenu = false;
				reset();
			}
	}
	else
	{
		if (stopFlow)
		{
			if (hoverPA)
			{
				reset();
			}
			else if (hoverQG)
			{
				mainMenu = true;
			}
		}
		else if (!postmortem1 && !postmortem3) {
			mouseInput.x = x;
			mouseInput.y = height - y;
			if (!pause)
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
					mouseLeft_down = false;
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
					mouseLeft_down = true;
			}
			else if (hoverFP)
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN &&
					game.points >= costsFirePower[game.fp] && game.fp < FIREPOWER)
				{
					state = GLUT_UP;
					consumePoints(costsFirePower[game.fp]);
					game.fp += 1;
					updateText(&textMenu.at(5), intToCharBuff(game.fp + 1));
					if (game.fp < FIREPOWER) {
						std::stringstream ss;
						ss << "UPGRADE FIREPOWER (COST " << costsFirePower[game.fp] << ")";
						textFirePower = stringToCharBuff(ss.str());
					}
					else
						textFirePower = stringToCharBuff("FIREPOWER FULLY UPGRADED");
				}
			}
			else if (hoverH)
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN &&
					game.points >= costsHealth[game.h] && game.h < HEALTH)
				{
					state = GLUT_UP;
					consumePoints(costsHealth[game.h]);
					game.h += 1;
					game.health += (MAX_HEALTH - INIT_HEALTH) / HEALTH;
					game.maxHealth += (MAX_HEALTH - INIT_HEALTH) / HEALTH;
					updateHealthBar(game.health);
					updateText(&textMenu.at(7), intToCharBuff(game.h + 1));
					if (game.h < HEALTH) {
						std::stringstream ss;
						ss << "UPGRADE HEALTH (COST " << costsHealth[game.h] << ")";
						textHealth = stringToCharBuff(ss.str());
					}
					else
						textHealth = stringToCharBuff("HEALTH FULLY UPGRADED");
				}
			}
			else if (hoverS)
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN &&
					game.points >= costsSpeed[game.s] && game.s < SPEED)
				{
					state = GLUT_UP;
					consumePoints(costsSpeed[game.s]);
					game.s += 1;
					vel_inc += 0.12;
					vel_dec += 0.1;
					max_vel += 2.0;
					updateText(&textMenu.at(9), intToCharBuff(game.s + 1));
					if (game.s < SPEED) {
						std::stringstream ss;
						ss << "UPGRADE SPEED (COST " << costsSpeed[game.s] << ")";
						textSpeed = stringToCharBuff(ss.str());
					}
					else
						textSpeed = stringToCharBuff("SPEED FULLY UPGRADED");
				}
			}
			else if (hoverL)
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && game.points >= costLife)
				{
					game.points -= costLife;
					game.lives++;
					updateText(&textStatsBar.at(1), intToCharBuff(game.lives));
					updateText(&textMenu.at(16), intToCharBuff(game.lives));
					std::stringstream ss;
					ss << "PURCHASE LIFE (COST " << costLife << ")";
					textLife = stringToCharBuff(ss.str());
				}
			}
			else if (hoverB)
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && game.points >= costBomb)
				{
					game.points -= costBomb;
					game.bombs++;
					updateText(&textStatsBar.at(3), intToCharBuff(game.bombs));
					updateText(&textMenu.at(14), intToCharBuff(game.bombs));
					std::stringstream ss;
					ss << "PURCHASE BOMB (COST " << costBomb << ")";
					textBomb = stringToCharBuff(ss.str());
				}
			}
			else if (hoverP[0])
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
				{
					if (!game.perksActive[0] && game.numActivePerks < game.maxPerks)
					{
						game.perksActive[0] = true;
						player.scale /= 3;
						game.numActivePerks++;
					}
					else if (game.perksActive[0])
					{
						game.perksActive[0] = false;
						player.scale *= 3;
						game.numActivePerks--;
					}

				}
			}
			else if (hoverP[1])
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
				{
					if (!game.perksActive[1] && game.numActivePerks < game.maxPerks)
					{
						game.perksActive[1] = true;
						game.numActivePerks++;
					}
					else if (game.perksActive[1])
					{
						game.perksActive[1] = false;
						game.numActivePerks--;
					}

				}
			}
			else if (hoverP[2])
			{
				if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
				{
					if (!game.perksActive[2] && game.numActivePerks < game.maxPerks)
					{
						game.perksActive[2] = true;
						game.numActivePerks++;
						game.cdmultiplier = 0.5;
					}
					else if (game.perksActive[2])
					{
						game.perksActive[2] = false;
						game.numActivePerks--;
						game.cdmultiplier = 1;
					}

				}
			}
			// if hoverP[3] hoverP[4] hoverP[5]
			else if (hoverC)
			{

			}
			else if (hoverPMM)
				mainMenu = true;
		}
	}
}

void keyboardPressedEvent(unsigned char key, int x, int y)
{
	if (mainMenu)
	{
		if (key == 27)
			exit(0);
	}
	else
	{
		if (!stopFlow)
		{
			switch (key)
			{
			case 'a':
				if (!postmortem1 && !postmortem3)
					pressing_left = true;
				break;
			case 'd':
				if (!postmortem1 && !postmortem3)
					pressing_right = true;
				break;
			case 'w':
				if (!postmortem1 && !postmortem3)
					pressing_up = true;
				break;
			case 's':
				if (!postmortem1 && !postmortem3)
					pressing_down = true;
				break;
			case 'e':
				reset();
				break;
			case 'r': // solo per finalità di test
				showTrails = !showTrails;
				/*
				if (gameOver)
				{
					gameOver = false;
				}
				else
				{
					scalaGameOver = 100.0;
					fattoreRiduzione = 0.01;
					progressiveTranspGO = 0.0;
					progressiveTranspPunteggio = 0.01;
					textGameOver = createText2(width * 60 / 1000, height * 638 / 1000, true, scalaGameOver, true, "GAME OVER", { 1.0, 1.0, 1.0, 0.0 });
					textPunteggio.clear();
					snprintf(buffer, 32, "TOTAL SCORE: %5d", game.score);
					textPunteggio.push_back(createText2(width * 280 / 1000, height * 400 / 1000, false, 10.0, true, buffer, { 1.0, 1.0, 1.0, 0.0 }));
					gameOver = true;
				}*/
				break;
			case 't': // solo per finalità di test
				playSoundEffect(3, "bomb");
				bombaPos = player.pos;
				sequenza_bomba = 1;
				break;
			case 'y': // solo per finalità di test
				if (game.fp < FIREPOWER)
				{
					game.fp++;
					updateText(&textMenu.at(5), intToCharBuff(game.fp + 1));
					if (game.fp < FIREPOWER) {
						std::stringstream ss;
						ss << "UPGRADE FIREPOWER (COST " << costsFirePower[game.fp] << ")";
						textFirePower = stringToCharBuff(ss.str());
					}
					else
						textFirePower = stringToCharBuff("FIREPOWER FULLY UPGRADED");
				}
				break;
			case 'u': // solo per finalità di test
				if (game.h < HEALTH)
				{
					game.h++;
					game.health += (MAX_HEALTH - INIT_HEALTH) / HEALTH;
					game.maxHealth += (MAX_HEALTH - INIT_HEALTH) / HEALTH;
					updateHealthBar(game.health);
					updateText(&textMenu.at(7), intToCharBuff(game.h + 1));
					if (game.h < HEALTH) {
						std::stringstream ss;
						ss << "UPGRADE HEALTH (COST " << costsHealth[game.h] << ")";
						textHealth = stringToCharBuff(ss.str());
					}
					else
						textHealth = stringToCharBuff("HEALTH FULLY UPGRADED");
				}
				break;
			case 'i': // solo per finalità di test
				if (game.h < HEALTH)
				{
					game.s++;
					updateText(&textMenu.at(9), intToCharBuff(game.s + 1));
					if (game.s < SPEED) {
						std::stringstream ss;
						ss << "UPGRADE SPEED (COST " << costsSpeed[game.s] << ")";
						textSpeed = stringToCharBuff(ss.str());
					}
					else
						textSpeed = stringToCharBuff("SPEED FULLY UPGRADED");
				}
				break;
			case 'o': // solo per finalità di test
				game.unlockedPerks++;
				unlockPerk();
				break;
			case 'p':
				if (!postmortem3 && !gameOver)
				{
					if (!pause)
						pauseStartTimestamp = glutGet(GLUT_ELAPSED_TIME);
					else
					{
						deltaPause += glutGet(GLUT_ELAPSED_TIME) - pauseStartTimestamp;
					}
					pause = !pause;
				}
				break;
			case '+':
				game.maxPerks++;
				break;
			case ' ':
				if (!postmortem1 && !postmortem3 && !pause)
					if (game.bombs > 0 && sequenza_bomba == 0)
					{
						playSoundEffect(3, "bomb");
						game.bombs--;
						updateText(&textStatsBar.at(3), intToCharBuff(game.bombs));
						updateText(&textMenu.at(14), intToCharBuff(game.bombs));
						bombaPos = player.pos;
						sequenza_bomba = 1;
					}
				break;
			default:
				break;
			}
		}
	}
}

void keyboardReleasedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
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

void mouseDragEvent(int x, int y)
{
	mouseInput.x = x;
	mouseInput.y = height - y;
}

void mousePassiveMotionEvent(int x, int y)
{
	if (mainMenu)
	{
		mouseInput.x = x;
		mouseInput.y = height - y;
		float x1 = width * 390 / 1000;
		float x2 = width * 610 / 1000;
		float y1 = height * 690 / 1000;
		float y2 = height * 810 / 1000;
		float xInc1 = 210;
		float xInc2 = 280;
		float yInc1 = 110;
		if (mouseInput.x > x1 && mouseInput.x < x2 && mouseInput.y > y1 && mouseInput.y < y2)
		{
			textMainMenu.push_back(createText2(width * 482 / 1000, height * 750 / 1000, false, 5.0, true, "PLAY", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 1.0 }, x1, x2, y1, y2);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 1.0 }, x1, x2, y1, y2);
			hoverMM[0] = true;
		}
		else if (mouseInput.x > x1 - xInc1 && mouseInput.x < x2 - xInc1 && mouseInput.y > y1 - yInc1 && mouseInput.y < y2 - yInc1)
		{
			textMainMenu.push_back(createText2(width * 270 / 1000, height * 598 / 1000, false, 5.0, true, "ACHIEVEMENTS", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 0.1 }, x1 - xInc1, x2 - xInc1, y1 - yInc1, y2 - yInc1);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.1 }, x1 - xInc1, x2 - xInc1, y1 - yInc1, y2 - yInc1);
			hoverMM[1] = true;
		}
		else if (mouseInput.x > x1 + xInc1 && mouseInput.x < x2 + xInc1 && mouseInput.y > y1 - yInc1 && mouseInput.y < y2 - yInc1)
		{
			textMainMenu.push_back(createText2(width * 605 / 1000, height * 598 / 1000, false, 5.0, true, "HIGH SCORES", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 0.1 }, x1 + xInc1, x2 + xInc1, y1 - yInc1, y2 - yInc1);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.1 }, x1 + xInc1, x2 + xInc1, y1 - yInc1, y2 - yInc1);
			hoverMM[2] = true;
		}
		else if (mouseInput.x > x1 - xInc2 && mouseInput.x < x2 - xInc2 && mouseInput.y > y1 - yInc1*2 && mouseInput.y < y2 - yInc1*2)
		{
			textMainMenu.push_back(createText2(width * 214 / 1000, height * 446 / 1000, false, 5.0, true, "INSTRUCTIONS", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 0.1 }, x1 - xInc2, x2 - xInc2, y1 - yInc1*2, y2 - yInc1*2);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.1 }, x1 - xInc2, x2 - xInc2, y1 - yInc1*2, y2 - yInc1*2);
			hoverMM[3] = true;
		}
		else if (mouseInput.x > x1 + xInc2 && mouseInput.x < x2 + xInc2 && mouseInput.y > y1 - yInc1 * 2 && mouseInput.y < y2 - yInc1 * 2)
		{
			textMainMenu.push_back(createText2(width * 660 / 1000, height * 446 / 1000, false, 5.0, true, "WALKTHROUGH", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 0.1 }, x1 + xInc2, x2 + xInc2, y1 - yInc1 * 2, y2 - yInc1 * 2);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.1 }, x1 + xInc2, x2 + xInc2, y1 - yInc1 * 2, y2 - yInc1 * 2);
			hoverMM[4] = true;
		}
		else if (mouseInput.x > x1 - xInc1 && mouseInput.x < x2 - xInc1 && mouseInput.y > y1 - yInc1 * 3 && mouseInput.y < y2 - yInc1 * 3)
		{
			textMainMenu.push_back(createText2(width * 280 / 1000, height * 294 / 1000, false, 5.0, true, "MORE GAMES", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 0.1 }, x1 - xInc1, x2 - xInc1, y1 - yInc1 * 3, y2 - yInc1 * 3);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.1 }, x1 - xInc1, x2 - xInc1, y1 - yInc1 * 3, y2 - yInc1 * 3);
			hoverMM[5] = true;
		}
		else if (mouseInput.x > x1 + xInc1 && mouseInput.x < x2 + xInc1 && mouseInput.y > y1 - yInc1 * 3 && mouseInput.y < y2 - yInc1 * 3)
		{
			textMainMenu.push_back(createText2(width * 623 / 1000, height * 294 / 1000, false, 5.0, true, "DOWNLOAD", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 0.1 }, x1 + xInc1, x2 + xInc1, y1 - yInc1 * 3, y2 - yInc1 * 3);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 0.1 }, x1 + xInc1, x2 + xInc1, y1 - yInc1 * 3, y2 - yInc1 * 3);
			hoverMM[6] = true;
		}
		else if (mouseInput.x > x1 && mouseInput.x < x2 && mouseInput.y > y1 - yInc1 * 4 && mouseInput.y < y2 - yInc1 * 4)
		{
			textMainMenu.push_back(createText2(width * 460 / 1000, height * 142 / 1000, false, 5.0, true, "CREDITS", { 1.0, 1.0, 1.0, 0.1 }));
			disegnaRettangolo(&disegnaMainMenu, &coloreMainMenu, { 0.0, 0.0, 1.0, 1.0 }, x1, x2, y1 - yInc1 * 4, y2 - yInc1 * 4);
			disegnaContornoRettangolo(&disegnaMainMenuL, &coloreMainMenuL, { 1.0, 1.0, 1.0, 1.0 }, x1, x2, y1 - yInc1 * 4, y2 - yInc1 * 4);
			hoverMM[7] = true;
		}
		else
		{
			textMainMenu.clear();
			disegnaMainMenu.clear();
			coloreMainMenu.clear();
			disegnaMainMenuL.clear();
			coloreMainMenuL.clear();
			initMainMenu();
			for (int i = 0; i < 8; i++)
				hoverMM[i] = false;
		}
	}
	else
	{
		if (!postmortem1)
		{
			mouseInput.x = x;
			mouseInput.y = height - y;
			if (pause)
			{
				//upgrades
				int x1 = width * 440 / 1000;
				int x2 = width * 458 / 1000;
				int y1 = height * 682 / 1000;
				int y2 = height * 714 / 1000;
				//triangles
				int x1_2 = width * 442 / 1000;
				int x2_2 = width * 456 / 1000;
				int x3_2 = (x1 + x2) / 2;
				int y1_2 = height * 688 / 1000;
				int y2_2 = height * 709 / 1000;
				//boxes
				int x1_3 = width * 170 / 1000;
				int x2_3 = width * 250 / 1000;
				int y1_3 = height * 355 / 1000;
				int y2_3 = height * 485 / 1000;
				//lucchetto
				int x1_4 = width * 185 / 1000;
				int x2_4 = width * 235 / 1000;
				int y1_4 = height * 375 / 1000;
				int y2_4 = height * 422 / 1000;
				//buys
				int x1_5 = width * 790 / 1000;
				int x2_5 = width * 808 / 1000;
				int y1_5 = height * 682 / 1000;
				int y2_5 = height * 714 / 1000;
				//crosses
				int x1_6 = width * 792 / 1000;
				int x2_6 = width * 807 / 1000;
				int y1_6 = height * 695 / 1000;
				int y2_6 = height * 702 / 1000;
				int x1_7 = width * 797 / 1000;
				int x2_7 = width * 802 / 1000;
				int y1_7 = height * 685 / 1000;
				int y2_7 = height * 712 / 1000;


				if (mouseInput.x > width * 440 / 1000 && mouseInput.x < width * 458 / 1000
					&& mouseInput.y > height * 682 / 1000 && mouseInput.y < height * 714 / 1000)
				{
					disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1, x2, y1, y2);
					disegnaTriangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_2, x2_2, x3_2, y1_2, y2_2);
					updateText(&textMenu.at(11), textFirePower);
					hoverFP = true;
				}
				else if (mouseInput.x > width * 440 / 1000 && mouseInput.x < width * 458 / 1000
					&& mouseInput.y >(height * 682 / 1000) - 30 && mouseInput.y < (height * 714 / 1000) - 30)
				{
					disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1, x2, y1 - 30, y2 - 30);
					disegnaTriangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_2, x2_2, x3_2, y1_2 - 30, y2_2 - 30);
					updateText(&textMenu.at(11), textHealth);
					hoverH = true;
				}
				else if (mouseInput.x > width * 440 / 1000 && mouseInput.x < width * 458 / 1000
					&& mouseInput.y >(height * 682 / 1000) - 60 && mouseInput.y < (height * 714 / 1000) - 60)
				{
					disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1, x2, y1 - 60, y2 - 60);
					disegnaTriangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_2, x2_2, x3_2, y1_2 - 60, y2_2 - 60);
					updateText(&textMenu.at(11), textSpeed);
					hoverS = true;
				}
				else if (mouseInput.x > width * 790 / 1000 && mouseInput.x < width * 808 / 1000
					&& mouseInput.y > height * 682 / 1000 && mouseInput.y < height * 714 / 1000)
				{
					disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_5, x2_5, y1_5, y2_5);
					disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_6, x2_6, y1_6, y2_6);
					disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_7, x2_7, y1_7, y2_7);
					updateText(&textMenu.at(11), textBomb);
					hoverB = true;
				}
				else if (mouseInput.x > width * 790 / 1000 && mouseInput.x < width * 808 / 1000
					&& mouseInput.y >(height * 682 / 1000) - 30 && mouseInput.y < (height * 714 / 1000) - 30)
				{
					disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_5, x2_5, y1_5 - 30, y2_5 - 30);
					disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_6, x2_6, y1_6 - 30, y2_6 - 30);
					disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_7, x2_7, y1_7 - 30, y2_7 - 30);
					updateText(&textMenu.at(11), textLife);
					hoverL = true;
				}
				else if (mouseInput.x > width * 170 / 1000 && mouseInput.x < width * 250 / 1000
					&& mouseInput.y > height * 355 / 1000 && mouseInput.y < height * 485 / 1000)
				{
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_3, x2_3, y1_3, y2_3);
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_4, x2_4, y1_4, y2_4);
					//disegnaCoronaCircolare(&disegnaHover, &coloreHover, { (float)(x1_4 + x2_4) / 2, (float)y2_4 },
					//	{ 0.0, 0.0, 0.0, 1.0f }, { 0.0, 0.0, 0.0, 1.0f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);
					if (game.unlockedPerks >= 1)
					{
						updateText(&textMenu.at(11), stringToCharBuff("REDUCED SIZE"));
						hoverP[0] = true;
					}
					else
						updateText(&textMenu.at(11), stringToCharBuff("LOCKED"));
				}
				else if (mouseInput.x > (width * 170 / 1000) + 130 && mouseInput.x < (width * 250 / 1000) + 130
					&& mouseInput.y > height * 355 / 1000 && mouseInput.y < height * 485 / 1000)
				{
					x1_3 += 130;
					x2_3 += 130;
					x1_4 += 130;
					x2_4 += 130;
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_3, x2_3, y1_3, y2_3);
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_4, x2_4, y1_4, y2_4);
					//disegnaCoronaCircolare(&disegnaHover, &coloreHover, { (float)(x1_4 + x2_4) / 2, (float)y2_4 },
					//	{ 0.0, 0.0, 0.0, 1.0f }, { 0.0, 0.0, 0.0, 1.0f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);
					if (game.unlockedPerks >= 2)
					{
						updateText(&textMenu.at(11), stringToCharBuff("BOUNCING BULLETS"));
						hoverP[1] = true;
					}
					else
						updateText(&textMenu.at(11), stringToCharBuff("LOCKED"));
				}
				else if (mouseInput.x > (width * 170 / 1000) + 260 && mouseInput.x < (width * 250 / 1000) + 260
					&& mouseInput.y > height * 355 / 1000 && mouseInput.y < height * 485 / 1000)
				{
					x1_3 += 260;
					x2_3 += 260;
					x1_4 += 260;
					x2_4 += 260;
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_3, x2_3, y1_3, y2_3);
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_4, x2_4, y1_4, y2_4);
					//disegnaCoronaCircolare(&disegnaHover, &coloreHover, { (float)(x1_4 + x2_4) / 2, (float)y2_4 },
					//	{ 0.0, 0.0, 0.0, 1.0f }, { 0.0, 0.0, 0.0, 1.0f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);
					if (game.unlockedPerks >= 3)
					{
						updateText(&textMenu.at(11), stringToCharBuff("DOUBLE TIME"));
						hoverP[2] = true;
					}
					else
						updateText(&textMenu.at(11), stringToCharBuff("LOCKED"));
				}
				else if (mouseInput.x > (width * 170 / 1000) && mouseInput.x < (width * 250 / 1000)
					&& mouseInput.y >(height * 355 / 1000) - 120 && mouseInput.y < (height * 485 / 1000) - 120)
				{
					y1_3 -= 120;
					y2_3 -= 120;
					y1_4 -= 120;
					y2_4 -= 120;
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_3, x2_3, y1_3, y2_3);
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_4, x2_4, y1_4, y2_4);
					//disegnaCoronaCircolare(&disegnaHover, &coloreHover, { (float)(x1_4 + x2_4) / 2, (float)y2_4 },
					//	{ 0.0, 0.0, 0.0, 1.0f }, { 0.0, 0.0, 0.0, 1.0f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);
					if (game.unlockedPerks >= 4)
					{
						updateText(&textMenu.at(11), stringToCharBuff("PENETRATING BULLETS"));
						hoverP[3] = true;
					}
					else
						updateText(&textMenu.at(11), stringToCharBuff("LOCKED"));
				}
				else if (mouseInput.x > (width * 170 / 1000) + 130 && mouseInput.x < (width * 250 / 1000) + 130
					&& mouseInput.y >(height * 355 / 1000) - 120 && mouseInput.y < (height * 485 / 1000) - 120)
				{
					x1_3 += 130;
					x2_3 += 130;
					x1_4 += 130;
					x2_4 += 130;
					y1_3 -= 120;
					y2_3 -= 120;
					y1_4 -= 120;
					y2_4 -= 120;
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_3, x2_3, y1_3, y2_3);
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_4, x2_4, y1_4, y2_4);
					//disegnaCoronaCircolare(&disegnaHover, &coloreHover, { (float)(x1_4 + x2_4) / 2, (float)y2_4 },
					//	{ 0.0, 0.0, 0.0, 1.0f }, { 0.0, 0.0, 0.0, 1.0f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);
					if (game.unlockedPerks >= 5)
					{
						updateText(&textMenu.at(11), stringToCharBuff("EXPLOSIVE BULLETS"));
						hoverP[4] = true;
					}
					else
						updateText(&textMenu.at(11), stringToCharBuff("LOCKED"));
				}
				else if (mouseInput.x > (width * 170 / 1000) + 260 && mouseInput.x < (width * 250 / 1000) + 260
					&& mouseInput.y >(height * 355 / 1000) - 120 && mouseInput.y < (height * 485 / 1000) - 120)
				{
					x1_3 += 260;
					x2_3 += 260;
					x1_4 += 260;
					x2_4 += 260;
					y1_3 -= 120;
					y2_3 -= 120;
					y1_4 -= 120;
					y2_4 -= 120;
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0f, 1.0f, 1.0f, 1.0f }, x1_3, x2_3, y1_3, y2_3);
					//disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0f, 0.0f, 0.0f, 1.0f }, x1_4, x2_4, y1_4, y2_4);
					//disegnaCoronaCircolare(&disegnaHover, &coloreHover, { (float)(x1_4 + x2_4) / 2, (float)y2_4 },
					//	{ 0.0, 0.0, 0.0, 1.0f }, { 0.0, 0.0, 0.0, 1.0f }, (float)width / 45, (float)width / 60, 30, PI, 0.0f);
					if (game.unlockedPerks >= 6)
					{
						updateText(&textMenu.at(11), stringToCharBuff("TIME ALTER"));
						hoverP[5] = true;
					}
					else
						updateText(&textMenu.at(11), stringToCharBuff("LOCKED"));
				}
				else if (mouseInput.x > width * 650 / 1000 && mouseInput.x < width * 810 / 1000
					&& mouseInput.y > height * 250 / 1000 && mouseInput.y < height * 330 / 1000)
				{
					//bottone Controls
					disegnaRettangolo(&disegnaHover, &coloreHover, { 0.0, 0.0, 1.0, 1.0 }, width * 650 / 1000, width * 810 / 1000, height * 250 / 1000, height * 330 / 1000);
					disegnaContornoRettangolo(&disegnaHoverL, &coloreHoverL, { 1.0, 1.0, 1.0, 1.0 }, width * 650 / 1000, width * 810 / 1000, height * 250 / 1000, height * 330 / 1000);
					updateText2(&textMenu.at(22), { 1.0, 1.0, 1.0, 1.0 }, 4.5, stringToCharBuff("CONTROLS"));
					hoverC = true;
				}
				else if (mouseInput.x > width * 650 / 1000 && mouseInput.x < width * 810 / 1000
					&& mouseInput.y >  height * 150 / 1000 && mouseInput.y < height * 230 / 1000)
				{
					disegnaRettangolo(&disegnaHover, &coloreHover, { 1.0, 0.0, 0.0, 1.0 }, width * 650 / 1000, width * 810 / 1000, height * 150 / 1000, height * 230 / 1000);
					disegnaContornoRettangolo(&disegnaHoverL, &coloreHoverL, { 1.0, 1.0, 1.0, 1.0 }, width * 650 / 1000, width * 810 / 1000, height * 150 / 1000, height * 230 / 1000);
					updateText2(&textMenu.at(23), { 1.0, 1.0, 1.0, 1.0 }, textMenu.at(23).scale, stringToCharBuff("MAIN MENU"));
					hoverPMM = true;
				}
				else
				{
					updateText2(&textMenu.at(22), { 1.0, 1.0, 1.0, 5.0 }, 4.5, stringToCharBuff("CONTROLS"));
					updateText2(&textMenu.at(23), { 1.0, 1.0, 1.0, 5.0 }, 4.5, stringToCharBuff("MAIN MENU"));
					disegnaHover.clear();
					coloreHover.clear();
					disegnaHoverL.clear();
					coloreHoverL.clear();
					updateText(&textMenu.at(11), stringToCharBuff(" "));
					hoverFP = false;
					hoverH = false;
					hoverS = false;
					hoverB = false;
					hoverL = false;
					hoverC = false;
					hoverPMM = false;
					for (int i = 0; i < PERKS; i++)
						hoverP[i] = false;
				}
			}
			else if (gameOver && stopFlow) // Hover bottoni GAMEOVER
			{
				if (mouseInput.x > width * 240 / 1000 && mouseInput.x < width * 450 / 1000
					&& mouseInput.y > height * 150 / 1000 && mouseInput.y < height * 280 / 1000)
				{
					disegnaBottoniGO.clear();
					coloreBottoniGO.clear();
					disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.0, 1.0, 0.0, 1.0 }, width * 240 / 1000, width * 450 / 1000, height * 150 / 1000, height * 280 / 1000);
					disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 1.0, 0.0, 0.0, 0.4 }, width * 550 / 1000, width * 760 / 1000, height * 150 / 1000, height * 280 / 1000);
					hoverPA = true;
				}
				else if (mouseInput.x > width * 550 / 1000 && mouseInput.x < width * 760 / 1000
					&& mouseInput.y > height * 150 / 1000 && mouseInput.y < height * 280 / 1000)
				{
					disegnaBottoniGO.clear();
					coloreBottoniGO.clear();
					disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.0, 1.0, 0.0, 0.4 }, width * 240 / 1000, width * 450 / 1000, height * 150 / 1000, height * 280 / 1000);
					disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 1.0, 0.0, 0.0, 1.0 }, width * 550 / 1000, width * 760 / 1000, height * 150 / 1000, height * 280 / 1000);
					hoverQG = true;
				}
				else
				{
					disegnaBottoniGO.clear();
					coloreBottoniGO.clear();
					disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 0.0, 1.0, 0.0, 0.4 }, width * 240 / 1000, width * 450 / 1000, height * 150 / 1000, height * 280 / 1000);
					disegnaRettangolo(&disegnaBottoniGO, &coloreBottoniGO, { 1.0, 0.0, 0.0, 0.4 }, width * 550 / 1000, width * 760 / 1000, height * 150 / 1000, height * 280 / 1000);
					hoverPA = false;
					hoverQG = false;
				}
			}
		}
	}
	glutPostRedisplay();
}

void disegnaLineeScia(std::vector<vec2> scia, std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colours, float bigRadius, float lowRadius, int numTrian, Vel4 vel)
{
	vec4 transp_col = { colours.r, colours.g, colours.b, 0.0f };
	float step = (PI * 2) / numTrian;

	//disegna cerchio
	if (scia.size() == 1)
	{
		for (int i = 0; i < numTrian; i++)
		{
			//ex
			vec2 p0 = { scia.at(0).x + bigRadius * cos((double)i * step), scia.at(0).y + bigRadius * sin((double)i * step) };
			resPts->push_back(p0);
			resCol->push_back(transp_col);

			vec2 p1 = { scia.at(0).x + bigRadius * cos((double)(i + 1) * step), scia.at(0).y + bigRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p1);
			resCol->push_back(transp_col);

			vec2 p2 = { scia.at(0).x + bigRadius * cos((double)(i + 1) * step), scia.at(0).y + bigRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p2);
			resCol->push_back(transp_col);

			//in
			vec2 p3 = { scia.at(0).x + 0 * cos((double)i * step), scia.at(0).y + 0 * sin((double)i * step) };
			resPts->push_back(p3);
			resCol->push_back(colours);

			vec2 p4 = { scia.at(0).x + 0 * cos((double)i * step), scia.at(0).y + 0 * sin((double)i * step) };
			resPts->push_back(p4);
			resCol->push_back(colours);

			vec2 p5 = { scia.at(0).x + 0 * cos((double)(i + 1) * step), scia.at(0).y + 0 * sin((double)(i + 1) * step) };
			resPts->push_back(p5);
			resCol->push_back(colours);
		}
	}

	// disegna scia composta
	if (scia.size() > 2)
	{
		//disegna primo semicerchio
		float angle = getAngle(scia.at(0), scia.at(1)) * 0.0174533;
		float start = numTrian / (PI * 2) * (getAngle(scia.at(0), scia.at(1)) * 0.0174533 + PI / 2);
		float end = numTrian / 2 + start;
		if (vel.up == 0.0 && vel.dx == 0.0 &&
			vel.dw == 0.0 && vel.sx == 0.0)
			end = numTrian + start;
		for (int i = start; i < end - 0.5; i++)
		{
			//ex
			vec2 p0 = { scia.at(0).x + bigRadius * cos((double)i * step), scia.at(0).y + bigRadius * sin((double)i * step) };
			resPts->push_back(p0);//0
			resCol->push_back(colours);

			vec2 p1 = { scia.at(0).x + bigRadius * cos((double)(i + 1) * step), scia.at(0).y + bigRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p1);//1
			resCol->push_back(colours);
			resPts->push_back(p1);//1
			resCol->push_back(colours);

			vec2 p3 = { scia.at(0).x, scia.at(0).y };
			resPts->push_back(p3);//3
			resCol->push_back(colours);
			resPts->push_back(p3);//3
			resCol->push_back(colours);
			resPts->push_back(p0);//0
			resCol->push_back(colours);
		}

		step = (bigRadius - lowRadius) / scia.size();
		vec2 midPoint = { (scia.at(0).x + scia.at(1).x) / 2, (scia.at(0).y + scia.at(1).y) / 2 };
		angle = angle - (PI / 2);
		float compX = (bigRadius * cos(angle));
		float compY = (bigRadius * sin(angle));
		vec2 upPoint;
		vec2 downPoint;
		vec2 lastUp = { midPoint.x + compX, midPoint.y + compY };
		vec2 lastDown = { midPoint.x - compX, midPoint.y - compY };
		int j = 1;
		for (int i = 0; i < scia.size() - 1; i+=2)
		{
			/*/if (getDistance(scia.at(i), scia.at(j)) < sogliaScia)
			{

			}*/
			if (i == 0) //prima iterazione +12punti
			{

				//first rectangle triangle
				//up
				resPts->push_back(scia.at(0));//60
				resCol->push_back(colours);
				resPts->push_back({ scia.at(0).x + compX, scia.at(0).y + compY });//61
				resCol->push_back(colours);
				resPts->push_back({ scia.at(0).x + compX, scia.at(0).y + compY });//61
				resCol->push_back(colours);
				resPts->push_back(lastUp);//62
				resCol->push_back(colours);
				resPts->push_back(lastUp);//62
				resCol->push_back(colours);
				resPts->push_back(scia.at(0));//60
				resCol->push_back(colours);
				//down
				resPts->push_back(scia.at(0));//63
				resCol->push_back(colours);
				resPts->push_back({ scia.at(0).x - compX, scia.at(0).y - compY });//64
				resCol->push_back(colours);
				resPts->push_back({ scia.at(0).x - compX, scia.at(0).y - compY });//64
				resCol->push_back(colours);
				resPts->push_back(lastDown);//65
				resCol->push_back(colours);
				resPts->push_back(lastDown);//65
				resCol->push_back(colours);
				resPts->push_back(scia.at(0));//63
				resCol->push_back(colours);

				//triangle
				//up
				resPts->push_back(scia.at(0));//66
				resCol->push_back(colours);
				resPts->push_back(scia.at(1));//67
				resCol->push_back(colours);
				resPts->push_back(scia.at(1));//67
				resCol->push_back(colours);
				resPts->push_back(lastUp);//68
				resCol->push_back(colours);
				resPts->push_back(lastUp);//68
				resCol->push_back(colours);
				resPts->push_back(scia.at(0));//66
				resCol->push_back(colours);
				//down
				resPts->push_back(scia.at(0));//69
				resCol->push_back(colours);
				resPts->push_back(scia.at(1));//70
				resCol->push_back(colours);
				resPts->push_back(scia.at(1));//70
				resCol->push_back(colours);
				resPts->push_back(lastDown);//71
				resCol->push_back(colours);
				resPts->push_back(lastDown);//71
				resCol->push_back(colours);
				resPts->push_back(scia.at(0));//69
				resCol->push_back(colours);
			}
			else
			{
				midPoint = { (scia.at(i).x + scia.at(i + 1).x) / 2, (scia.at(i).y + scia.at(i + 1).y) / 2 };
				angle = getAngle(scia.at(i), scia.at(i + 1)) * 0.0174533 - (PI / 2);
				compX = (bigRadius - (step * i)) * cos(angle);
				compY = (bigRadius - (step * i)) * sin(angle);
				upPoint = { midPoint.x + compX, midPoint.y + compY };
				downPoint = { midPoint.x - compX, midPoint.y - compY };

				//first triangle
				//up
				resPts->push_back(lastUp);//18+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i));//19+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i));//19+i*12
				resCol->push_back(colours);
				resPts->push_back(upPoint);//20+i*12
				resCol->push_back(colours);
				resPts->push_back(upPoint);//20+i*12
				resCol->push_back(colours);
				resPts->push_back(lastUp);//18+i*12
				resCol->push_back(colours);
				//down
				resPts->push_back(lastDown);//21+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i));//22+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i));//22+i*12
				resCol->push_back(colours);
				resPts->push_back(downPoint);//23+i*12
				resCol->push_back(colours);
				resPts->push_back(downPoint);//23+i*12
				resCol->push_back(colours);
				resPts->push_back(lastDown);//21+i*12
				resCol->push_back(colours);

				//second triangle
				//up
				resPts->push_back(scia.at(i));//24+i*12
				resCol->push_back(colours);
				resPts->push_back(upPoint);//25+i*12
				resCol->push_back(colours);
				resPts->push_back(upPoint);//25+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i + 1));//26+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i + 1));//26+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i));//24+i*12
				resCol->push_back(colours);
				//down
				resPts->push_back(scia.at(i));//27+i*12
				resCol->push_back(colours);
				resPts->push_back(downPoint);//28+i*12
				resCol->push_back(colours);
				resPts->push_back(downPoint);//28+i*12
				resCol->push_back(colours);
				resPts->push_back({ scia.at(i + 1) });//29+i*12
				resCol->push_back(colours);
				resPts->push_back({ scia.at(i + 1) });//29+i*12
				resCol->push_back(colours);
				resPts->push_back(scia.at(i));//27+i*12
				resCol->push_back(colours);

				lastUp = upPoint;
				lastDown = downPoint;
			}
		}
		//last rectangle triangle
		//up
		resPts->push_back(upPoint);
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1).x + compX,  scia.at(scia.size() - 1).y + compY });
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1).x + compX,  scia.at(scia.size() - 1).y + compY });
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1) });
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1) });
		resCol->push_back(colours);
		resPts->push_back(upPoint);
		resCol->push_back(colours);
		//down
		resPts->push_back(downPoint);
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1).x - compX,  scia.at(scia.size() - 1).y - compY });
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1).x - compX,  scia.at(scia.size() - 1).y - compY });
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1) });
		resCol->push_back(colours);
		resPts->push_back({ scia.at(scia.size() - 1) });
		resCol->push_back(colours);
		resPts->push_back(downPoint);
		resCol->push_back(colours);


		//disegna secondo semicerchio
		step = (PI * 2) / numTrian;
		start = numTrian / (PI * 2) * angle;
		end = numTrian / 2 + start;
		if (vel.up == 0.0 && vel.dx == 0.0 &&
			vel.dw == 0.0 && vel.sx == 0.0)
			end = start;
		for (int i = start; i < end - 0.5; i++)
		{
			//ex
			vec2 p0 = { scia.at(scia.size() - 1).x + lowRadius * cos((double)i * step), scia.at(scia.size() - 1).y + lowRadius * sin((double)i * step) };
			resPts->push_back(p0);
			resCol->push_back(colours);

			vec2 p1 = { scia.at(scia.size() - 1).x + lowRadius * cos((double)(i + 1) * step), scia.at(scia.size() - 1).y + lowRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p1);
			resCol->push_back(colours);
			resPts->push_back(p1);
			resCol->push_back(colours);

			//in
			vec2 p3 = { scia.at(scia.size() - 1).x, scia.at(scia.size() - 1).y };
			resPts->push_back(p3);
			resCol->push_back(colours);
			resPts->push_back(p3);
			resCol->push_back(colours);
			resPts->push_back(p0);
			resCol->push_back(colours);
		}
	}
}

void disegnaScia(std::vector<vec2> scia, std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colours, float bigRadius, float lowRadius, int numTrian, Vel4 vel)
{
	vec4 transp_col = { colours.r, colours.g, colours.b, 0.0f };
	float step = (PI * 2) / numTrian;

	//disegna cerchio
	if (scia.size() == 1)
	{
		for (int i = 0; i < numTrian; i++)
		{
			//ex
			vec2 p0 = { scia.at(0).x + bigRadius * cos((double)i * step), scia.at(0).y + bigRadius * sin((double)i * step) };
			resPts->push_back(p0);
			resCol->push_back(transp_col);

			vec2 p1 = { scia.at(0).x + bigRadius * cos((double)(i + 1) * step), scia.at(0).y + bigRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p1);
			resCol->push_back(transp_col);

			vec2 p2 = { scia.at(0).x + bigRadius * cos((double)(i + 1) * step), scia.at(0).y + bigRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p2);
			resCol->push_back(transp_col);

			//in
			vec2 p3 = { scia.at(0).x + 0 * cos((double)i * step), scia.at(0).y + 0 * sin((double)i * step) };
			resPts->push_back(p3);
			resCol->push_back(colours);

			vec2 p4 = { scia.at(0).x + 0 * cos((double)i * step), scia.at(0).y + 0 * sin((double)i * step) };
			resPts->push_back(p4);
			resCol->push_back(colours);

			vec2 p5 = { scia.at(0).x + 0 * cos((double)(i + 1) * step), scia.at(0).y + 0 * sin((double)(i + 1) * step) };
			resPts->push_back(p5);
			resCol->push_back(colours);
		}
	}

	// disegna scia composta
	if (scia.size() > 2)
	{
		//disegna primo semicerchio
		float angle = getAngle(scia.at(0), scia.at(1)) * 0.0174533;
		float start = numTrian / (PI * 2) * (getAngle(scia.at(0), scia.at(1)) * 0.0174533 + PI / 2);
		float end = numTrian / 2 + start;
		if (vel.up == 0.0 && vel.dx == 0.0 &&
			vel.dw == 0.0 && vel.sx == 0.0)
			end = numTrian + start;
		for (int i = start; i < end - 0.5; i++)
		{
			//ex
			vec2 p0 = { scia.at(0).x + bigRadius * cos((double)i * step), scia.at(0).y + bigRadius * sin((double)i * step) };
			resPts->push_back(p0);//0
			resCol->push_back(transp_col);

			vec2 p1 = { scia.at(0).x + bigRadius * cos((double)(i + 1) * step), scia.at(0).y + bigRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p1);//1
			resCol->push_back(transp_col);

			vec2 p3 = { scia.at(0).x, scia.at(0).y };
			resPts->push_back(p3);//3
			resCol->push_back(colours);
		}

		step = (bigRadius - lowRadius) / scia.size();
		vec2 midPoint = { (scia.at(0).x + scia.at(1).x) / 2, (scia.at(0).y + scia.at(1).y) / 2 };
		angle = angle - (PI / 2);
		float compX = (bigRadius * cos(angle));
		float compY = (bigRadius * sin(angle));
		vec2 upPoint;
		vec2 downPoint;
		vec2 lastUp = { midPoint.x + compX, midPoint.y + compY };
		vec2 lastDown = { midPoint.x - compX, midPoint.y - compY };
		int j = 1;
		for (int i = 0; i < scia.size() - 1; i++)
		{
			/*/if (getDistance(scia.at(i), scia.at(j)) < sogliaScia)
			{

			}*/
			if (i == 0) //prima iterazione +12punti
			{

				//first rectangle triangle
				//up
				resPts->push_back(scia.at(0));//60
				resCol->push_back(colours);
				resPts->push_back({ scia.at(0).x + compX, scia.at(0).y + compY });//61
				resCol->push_back(transp_col);
				resPts->push_back(lastUp);//62
				resCol->push_back(transp_col);
				//down
				resPts->push_back(scia.at(0));//63
				resCol->push_back(colours);
				resPts->push_back({ scia.at(0).x - compX, scia.at(0).y - compY });//64
				resCol->push_back(transp_col);
				resPts->push_back(lastDown);//65
				resCol->push_back(transp_col);

				//triangle
				//up
				resPts->push_back(scia.at(0));//66
				resCol->push_back(colours);
				resPts->push_back(scia.at(1));//67
				resCol->push_back(colours);
				resPts->push_back(lastUp);//68
				resCol->push_back(transp_col);
				//down
				resPts->push_back(scia.at(0));//69
				resCol->push_back(colours);
				resPts->push_back(scia.at(1));//70
				resCol->push_back(colours);
				resPts->push_back(lastDown);//71
				resCol->push_back(transp_col);
			}
			else
			{
				midPoint = { (scia.at(i).x + scia.at(i + 1).x) / 2, (scia.at(i).y + scia.at(i + 1).y) / 2 };
				angle = getAngle(scia.at(i), scia.at(i + 1)) * 0.0174533 - (PI / 2);
				compX = (bigRadius - (step * i)) * cos(angle);
				compY = (bigRadius - (step * i)) * sin(angle);
				upPoint = { midPoint.x + compX, midPoint.y + compY };
				downPoint = { midPoint.x - compX, midPoint.y - compY };

				//first triangle
				//up
				resPts->push_back(lastUp);//18+i*12
				resCol->push_back(transp_col);
				resPts->push_back(scia.at(i));//19+i*12
				resCol->push_back(colours);
				resPts->push_back(upPoint);//20+i*12
				resCol->push_back(transp_col);
				//down
				resPts->push_back(lastDown);//21+i*12
				resCol->push_back(transp_col);
				resPts->push_back(scia.at(i));//22+i*12
				resCol->push_back(colours);
				resPts->push_back(downPoint);//23+i*12
				resCol->push_back(transp_col);

				//second triangle
				//up
				resPts->push_back(scia.at(i));//24+i*12
				resCol->push_back(colours);
				resPts->push_back(upPoint);//25+i*12
				resCol->push_back(transp_col);
				resPts->push_back(scia.at(i + 1));//26+i*12
				resCol->push_back(colours);
				//down
				resPts->push_back(scia.at(i));//27+i*12
				resCol->push_back(colours);
				resPts->push_back(downPoint);//28+i*12
				resCol->push_back(transp_col);
				resPts->push_back({ scia.at(i + 1) });//29+i*12
				resCol->push_back(colours);

				lastUp = upPoint;
				lastDown = downPoint;
			}
		}
		//last rectangle triangle
		//up
		resPts->push_back(upPoint);
		resCol->push_back(transp_col);
		resPts->push_back({ scia.at(scia.size() - 1).x + compX,  scia.at(scia.size() - 1).y + compY });
		resCol->push_back(transp_col);
		resPts->push_back({ scia.at(scia.size() - 1) });
		resCol->push_back(colours);
		//down
		resPts->push_back(downPoint);
		resCol->push_back(transp_col);
		resPts->push_back({ scia.at(scia.size() - 1).x - compX,  scia.at(scia.size() - 1).y - compY });
		resCol->push_back(transp_col);
		resPts->push_back({ scia.at(scia.size() - 1) });
		resCol->push_back(colours);


		//disegna secondo semicerchio
		step = (PI * 2) / numTrian;
		start = numTrian / (PI * 2) * angle;
		end = numTrian / 2 + start;
		if (vel.up == 0.0 && vel.dx == 0.0 &&
			vel.dw == 0.0 && vel.sx == 0.0)
			end = start;
		for (int i = start; i < end - 0.5; i++)
		{
			//ex
			vec2 p0 = { scia.at(scia.size() - 1).x + lowRadius * cos((double)i * step), scia.at(scia.size() - 1).y + lowRadius * sin((double)i * step) };
			resPts->push_back(p0);
			resCol->push_back(transp_col);

			vec2 p1 = { scia.at(scia.size() - 1).x + lowRadius * cos((double)(i + 1) * step), scia.at(scia.size() - 1).y + lowRadius * sin((double)(i + 1) * step) };
			resPts->push_back(p1);
			resCol->push_back(transp_col);

			//in
			vec2 p3 = { scia.at(scia.size() - 1).x, scia.at(scia.size() - 1).y };
			resPts->push_back(p3);
			resCol->push_back(colours);
		}
	}

}

void disegnaCoronaCircolare(std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec2 center, vec4 coloursInt, vec4 coloursExt, float inRadius, float exRadius, int numTrian, float degreeRange, float degreeOffset)
{

	float step = (PI * 2) / numTrian;
	float divider = (PI * 2) / degreeRange;
	float start = numTrian / (PI * 2) * degreeOffset;

	for (int i = start; i < (numTrian / divider + start); i++)
	{
		//ex
		vec2 p0 = { center.x + exRadius * cos((double)i * step), center.y + exRadius * sin((double)i * step) };
		resPts->push_back(p0);
		resCol->push_back(coloursExt);

		vec2 p1 = { center.x + exRadius * cos((double)(i + 1) * step), center.y + exRadius * sin((double)(i + 1) * step) };
		resPts->push_back(p1);
		resCol->push_back(coloursExt);

		//in
		vec2 p2 = { center.x + inRadius * cos((double)i * step), center.y + inRadius * sin((double)i * step) };
		resPts->push_back(p2);
		resCol->push_back(coloursInt);

		vec2 p5 = { center.x + inRadius * cos((double)i * step), center.y + inRadius * sin((double)i * step) };
		resPts->push_back(p5);
		resCol->push_back(coloursInt);

		vec2 p4 = { center.x + exRadius * cos((double)(i + 1) * step), center.y + exRadius * sin((double)(i + 1) * step) };
		resPts->push_back(p4);
		resCol->push_back(coloursExt);

		vec2 p3 = { center.x + inRadius * cos((double)(i + 1) * step), center.y + inRadius * sin((double)(i + 1) * step) };
		resPts->push_back(p3);
		resCol->push_back(coloursInt);
	}

}

void disegnaCirconferenza(std::vector<vec2>* resPts, std::vector<vec4>* resCol, vec4 colour, vec2 center, float radius, int numSeg, float degreeRange, float degreeOffset, bool type)
{
	// step è quanto è lungo il segmento (la circonferenza avrà un numero di segmenti pari a num_segmenti)

	float step = (PI * 2) / numSeg;
	float divider = (PI * 2) / degreeRange;
	float start = numSeg / (PI * 2) * degreeOffset;

	for (int i = 0; i <= numSeg; i++)
	{
		vec2 p2 = {
			center.x + radius * cos(i * step),
			center.y + radius * sin(i * step) };

		// aggiungo il punto al vettore (push back aggiunge in coda e aumenta la dimensione di 1)
		if (type)
		{
			resPts->push_back(p2);
			resCol->push_back(colour);
		}
		resPts->push_back(p2);
		resCol->push_back(colour);
	}
}

void disegna_circonferenza(Navicella* navicella, vec2 centro, float raggio, vec4 colore)
{
	// step è quanto è lungo il segmento (la circonferenza avrà un numero di segmenti pari a num_segmenti)
	float step = (PI * 2) / num_segmenti;

	for (int i = 0; i <= num_segmenti; i++)
	{
		vec2 p2 = {
			centro.x + raggio * cos(i * step),
			centro.y + raggio * sin(i * step) };

		// aggiungo il punto al vettore (push back aggiunge in coda e aumenta la dimensione di 1)
		navicella->pts.push_back(p2);
		navicella->colors.push_back(colore);
	}
}

void my_disegna_navicella(Navicella* navicella)
{
	//player.points.push_back({ 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f });
	player.pts.push_back({ 0.0f, 0.0f });
	player.colors.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
	disegna_circonferenza(&player, { 0.0f, 0.0f }, nav_raggio, { 1.0f, 1.0f, 1.0f, 1.0f });
	//player.points.push_back({ 1.1f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f });
	player.pts.push_back({ 1.1f, 0.0f });
	player.colors.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
}

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";
	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}

void init(void)
{
	postmortem2 = true;
	postmortem_c = INVULNERABILITY * 3 / 5;

	initStages();

	game.stage = 0;
	game.waveKills = 0;
	game.stageKills = 0;
	game.currentGroup = 0;
	game.currentWave = 0;
	game.currentStage = 0;
	game.score = 0;
	game.unlockedPerks = 0;
	game.lives = 3;
	game.bombs = 2;
	game.health = 58;
	game.maxHealth = INIT_HEALTH;
	game.fp = 0;
	game.h = 0;
	game.s = 0;
	game.waveInitTimestamp = glutGet(GLUT_ELAPSED_TIME);
	game.cdmultiplier = 1;
	std::cout << "\ngame.waveInitTime: " << game.waveInitTimestamp << "\n";
	menuInit();
	statsBarInit();
	gameOverInit();
	initMainMenu();
	quadratiPerkAttiveInit();
	for (int i = 0; i < PERKS; i++)
		hoverP[i] = false;
	for (int i = 0; i < PERKS; i++)
		game.perksActive[i] = false;
	game.unlockedPerks = 0;
	game.numActivePerks = 0;
	game.maxPerks = 1; // aumenta con il tempo (fino a max 3)

	//setup text menuHovers
	std::stringstream ss;
	ss << "UPGRADE FIREPOWER (COST " << costsFirePower[0] << ")";
	textFirePower = stringToCharBuff(ss.str());
	ss.clear();
	ss.str(std::string());
	ss << "UPGRADE HEALTH (COST " << costsHealth[0] << ")";
	textHealth = stringToCharBuff(ss.str());
	ss.clear();
	ss.str(std::string());
	ss << "UPGRADE SPEED (COST " << costsSpeed[0] << ")";
	textSpeed = stringToCharBuff(ss.str());
	ss.clear();
	ss.str(std::string());
	ss << "PURCHASE BOMB (COST " << costBomb << ")";
	textBomb = stringToCharBuff(ss.str());
	ss.clear();
	ss.str(std::string());
	ss << "PURCHASE LIFE (COST " << costLife << ")";
	textLife = stringToCharBuff(ss.str());

	player.vel = { 0.0f, 0.0f };

	//Disegno SPAZIO
	glGenVertexArrays(1, &VAO_SPACE);
	glBindVertexArray(VAO_SPACE);
	glGenBuffers(1, &VBO_S);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
	glBufferData(GL_ARRAY_BUFFER, vertices_space * sizeof(Point), &Space[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Genero un VAO mainMenu
	glGenVertexArrays(1, &VAO_MM);
	glBindVertexArray(VAO_MM);
	//vertici
	glGenBuffers(1, &VBO_MM);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_MM);
	//colori
	glGenBuffers(1, &VBO_MMC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_MMC);

	//Genero un VAO mainMenuLines
	glGenVertexArrays(1, &VAO_MML);
	glBindVertexArray(VAO_MML);
	//vertici
	glGenBuffers(1, &VBO_MML);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_MML);
	//colori
	glGenBuffers(1, &VBO_MMLC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_MMLC);

	//Genero un VAO proiettili
	glGenVertexArrays(1, &VAO_P);
	glBindVertexArray(VAO_P);
	//vertici
	glGenBuffers(1, &VBO_P);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_P);
	//colori
	glGenBuffers(1, &VBO_PC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_PC);

	//Genero un VAO bomba
	glGenVertexArrays(1, &VAO_BMB);
	glBindVertexArray(VAO_BMB);
	//vertici
	glGenBuffers(1, &VBO_BMB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BMB);
	//colori
	glGenBuffers(1, &VBO_BMBC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BMBC);

	//Genero un VAO menu
	glGenVertexArrays(1, &VAO_M);
	glBindVertexArray(VAO_M);
	//vertici
	glGenBuffers(1, &VBO_M);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_M);
	//colori
	glGenBuffers(1, &VBO_MC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_MC);

	//Genero un VAO statsBar
	glGenVertexArrays(1, &VAO_SB);
	glBindVertexArray(VAO_SB);
	//vertici
	glGenBuffers(1, &VBO_SB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_SB);
	//colori
	glGenBuffers(1, &VBO_SBC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_SBC);

	//Genero un VAO healthBar
	glGenVertexArrays(1, &VAO_HB);
	glBindVertexArray(VAO_HB);
	//vertici
	glGenBuffers(1, &VBO_HB);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_HB);
	//colori
	glGenBuffers(1, &VBO_HBC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_HBC);

	//Genero un VAO hover triangoli
	glGenVertexArrays(1, &VAO_H);
	glBindVertexArray(VAO_H);
	//vertici
	glGenBuffers(1, &VBO_H);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_H);
	//colori
	glGenBuffers(1, &VBO_HC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_HC);

	//Genero un VAO hover linee
	glGenVertexArrays(1, &VAO_HL);
	glBindVertexArray(VAO_HL);
	//vertici
	glGenBuffers(1, &VBO_HL);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_HL);
	//colori
	glGenBuffers(1, &VBO_HLC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_HLC);

	//Genero un VAO perks
	glGenVertexArrays(1, &VAO_DP);
	glBindVertexArray(VAO_DP);
	//vertici
	glGenBuffers(1, &VBO_DP);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_DP);
	//colori
	glGenBuffers(1, &VBO_DPC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_DPC);

	//Genero un VAO perksTriangoli
	glGenVertexArrays(1, &VAO_DPT);
	glBindVertexArray(VAO_DPT);
	//vertici
	glGenBuffers(1, &VBO_DPT);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_DPT);
	//colori
	glGenBuffers(1, &VBO_DPTC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_DPTC);

	//Genero un VAO perksSelected
	glGenVertexArrays(1, &VAO_PS);
	glBindVertexArray(VAO_PS);
	//vertici
	glGenBuffers(1, &VBO_PS);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_PS);
	//colori
	glGenBuffers(1, &VBO_PSC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_PSC);

	//Genero un VAO test
	glGenVertexArrays(1, &VAO_TP);
	glBindVertexArray(VAO_TP);
	//vertici
	glGenBuffers(1, &VBO_TP);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_TP);
	//colori
	glGenBuffers(1, &VBO_TPC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_TPC);

	//Genero un VAO per bottoni GameOver
	glGenVertexArrays(1, &VAO_BGO);
	glBindVertexArray(VAO_BGO);
	//vertici
	glGenBuffers(1, &VBO_BGO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BGO);
	//colori
	glGenBuffers(1, &VBO_BGOC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_BGOC);


	//Genero un VAO per i contorni dei bottoni GameOver
	glGenVertexArrays(1, &VAO_CBGO);
	glBindVertexArray(VAO_CBGO);
	//vertici
	glGenBuffers(1, &VBO_CBGO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_CBGO);
	//colori
	glGenBuffers(1, &VBO_CBGOC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_CBGOC);


	//Definisco il colore che verrà assegnato allo sfondo della finestra
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// CREO LA STRUTTURA DELLA MIA NUOVA NAVICELLA
	player = {};
	player.pos = { width / 2, height / 2 };
	player.angle = PI / 2;
	player.scale = scala;

	// inserisco i punti nell'array della navicella per il successivo draw
	my_disegna_navicella(&player);

	// GENERO VAO e VBO DELLA NAVICELLA e BINDO
	glGenVertexArrays(1, &player.VAO);
	glBindVertexArray(player.VAO);
	glGenBuffers(1, &player.VBO_Geom);
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO_Geom);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &player.VBO_Col);
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO_Col);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Genero un VAO scie
	glGenVertexArrays(1, &player.VAO_S);
	glBindVertexArray(player.VAO_S);
	//vertici
	glGenBuffers(1, &player.VBO_S);
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO_S);
	//colori
	glGenBuffers(1, &player.VBO_SC);
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO_SC);

	//Genero un VAO per i residui delle scie dei nemici
	glGenVertexArrays(1, &VAO_RS);
	glBindVertexArray(VAO_RS);
	//vertici
	glGenBuffers(1, &VBO_RS);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_RS);
	//colori
	glGenBuffers(1, &VBO_RSC);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_RSC);




	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
}

void drawScene(void)
{
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programId);

	if (mainMenu)
	{
		//disegna triangoli mainMenu
		Model = mat4(1.0);
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_MM);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_MM);
		glBufferData(GL_ARRAY_BUFFER, disegnaMainMenu.size() * sizeof(vec2), disegnaMainMenu.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_MMC);
		glBufferData(GL_ARRAY_BUFFER, coloreMainMenu.size() * sizeof(vec4), coloreMainMenu.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glLineWidth(2.0f);
		glDrawArrays(GL_TRIANGLES, 0, disegnaMainMenu.size());
		glBindVertexArray(0);

		//disegna linee mainMenu
		Model = mat4(1.0);
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_MML);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_MML);
		glBufferData(GL_ARRAY_BUFFER, disegnaMainMenuL.size() * sizeof(vec2), disegnaMainMenuL.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_MMLC);
		glBufferData(GL_ARRAY_BUFFER, coloreMainMenuL.size() * sizeof(vec4), coloreMainMenuL.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glLineWidth(2.0f);
		glDrawArrays(GL_LINES, 0, disegnaMainMenuL.size());
		glBindVertexArray(0);

		//disegna testo mainMenu
		for (int i = 0; i < textMainMenu.size(); i++)
		{
			Text text = textMainMenu.at(i);

			if (text.visible)
			{
				for (int j = 0; j < text.letters.size(); j++)
				{
					Letter let = text.letters.at(j);

					glm::mat4 modelMatrix = glm::mat4(1.0);
					modelMatrix = translate(modelMatrix, glm::vec3(text.pos.x, text.pos.y, 0.0f));
					modelMatrix = scale(modelMatrix, glm::vec3(text.scale, text.scale, 0.0f));
					glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(modelMatrix));

					glBindVertexArray(let.VAO);
					glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Geom);
					glBufferData(GL_ARRAY_BUFFER, let.points.size() * sizeof(vec2), let.points.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Col);
					glBufferData(GL_ARRAY_BUFFER, let.colors.size() * sizeof(vec4), let.colors.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(1);
					if (let.sizePoints > 0.0f)
						glPointSize(let.sizePoints);
					else glPointSize(1.0f);
					if (let.widthLines > 0.0f)
						glLineWidth(let.widthLines);
					else glLineWidth(1.0f);

					glDrawArrays(let.drawMode, 0, let.points.size());

					glBindVertexArray(0);
				}
			}
		}
	}
	else
	{
		Model = mat4(1.0);
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_TP);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_TP);
		glBufferData(GL_ARRAY_BUFFER, testingPoints.size() * sizeof(vec2), testingPoints.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_TPC);
		glBufferData(GL_ARRAY_BUFFER, testingColore.size() * sizeof(vec4), testingColore.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glLineWidth(2.0f);
		glDrawArrays(GL_TRIANGLES, 0, testingPoints.size());
		glBindVertexArray(0);

		// disegna navicella
		if (disegnaNav) { // l'if serve per creare l'effetto intermittenza quando si viene colpiti (vita persa)
			Model = mat4(1.0);
			Model = translate(Model, vec3(player.pos.x, player.pos.y, 0.0));
			Model = scale(Model, vec3(player.scale, player.scale, 1.0));
			Model = rotate(Model, player.angle, vec3(0.0, 0.0, 1.0));
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(player.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, player.VBO_Geom);
			glBufferData(GL_ARRAY_BUFFER, player.pts.size() * sizeof(vec2), player.pts.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, player.VBO_Col);
			glBufferData(GL_ARRAY_BUFFER, player.colors.size() * sizeof(vec4), player.colors.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_LINE_STRIP, 0, player.pts.size() * 2);
			glBindVertexArray(0);

			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(player.VAO_S);
			glBindBuffer(GL_ARRAY_BUFFER, player.VBO_S);
			glBufferData(GL_ARRAY_BUFFER, player.puntiScia.size() * sizeof(vec2), player.puntiScia.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, player.VBO_SC);
			glBufferData(GL_ARRAY_BUFFER, player.coloreScia.size() * sizeof(vec4), player.coloreScia.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, player.puntiScia.size());
			glBindVertexArray(0);
		}

		// disegna proiettili
		Model = mat4(1.0);
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_P);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_P);
		glBufferData(GL_ARRAY_BUFFER, disegnaProiettili.size() * sizeof(vec2), disegnaProiettili.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_PC);
		glBufferData(GL_ARRAY_BUFFER, coloriProiettili.size() * sizeof(vec4), coloriProiettili.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glLineWidth(scalaProiettile);
		glDrawArrays(GL_LINES, 0, proiettili.size() * 2);
		glBindVertexArray(0);

		//disegna nemici
		for (int i = 0; i < nemici.size(); i++)
		{
			if (nemici.at(i).type == 4)
			{
				Model = mat4(1.0);
				Model = translate(Model, vec3(nemici.at(i).nav.pos, 0.0f));
				Model = scale(Model, vec3(nemici.at(i).nav.scale, nemici.at(i).nav.scale, 1.0));
				glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
				glBindVertexArray(nemici.at(i).nav.VAO);
				glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Geom);
				glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.pts.size() * sizeof(vec2), nemici.at(i).nav.pts.data(), GL_STATIC_DRAW);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Col);
				glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.colors.size() * sizeof(vec4), nemici.at(i).nav.colors.data(), GL_STATIC_DRAW);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);
				glLineWidth(2.0f);
				glDrawArrays(GL_LINES, 0, nemici.at(i).nav.pts.size());
				glBindVertexArray(0);
			}
			else
			{
				Model = mat4(1.0);
				Model = translate(Model, vec3(nemici.at(i).nav.pos, 0.0f));
				Model = scale(Model, vec3(nemici.at(i).nav.scale, nemici.at(i).nav.scale, 1.0));
				glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
				glBindVertexArray(nemici.at(i).nav.VAO);
				glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Geom);
				glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.pts.size() * sizeof(vec2), nemici.at(i).nav.pts.data(), GL_STATIC_DRAW);
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_Col);
				glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.colors.size() * sizeof(vec4), nemici.at(i).nav.colors.data(), GL_STATIC_DRAW);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);
				glLineWidth(2.0f);
				glDrawArrays(GL_LINE_STRIP, 0, nemici.at(i).nav.pts.size());
				glBindVertexArray(0);
			}

			//disegna scia nemico
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(nemici.at(i).nav.VAO_S);
			glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_S);
			glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.puntiScia.size() * sizeof(vec2), nemici.at(i).nav.puntiScia.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_SC);
			glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.coloreScia.size() * sizeof(vec4), nemici.at(i).nav.coloreScia.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, nemici.at(i).nav.puntiScia.size());
			glBindVertexArray(0);			

			//disegna scia nemico
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(nemici.at(i).nav.VAO_S);
			glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_S);
			glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.puntiScia2.size() * sizeof(vec2), nemici.at(i).nav.puntiScia2.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, nemici.at(i).nav.VBO_SC);
			glBufferData(GL_ARRAY_BUFFER, nemici.at(i).nav.coloreScia2.size() * sizeof(vec4), nemici.at(i).nav.coloreScia2.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_LINES, 0, nemici.at(i).nav.puntiScia2.size());
			glBindVertexArray(0);

		}

		for (int i = 0; i < residui.size(); i++)
		{
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_RS);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_RS);
			glBufferData(GL_ARRAY_BUFFER, residui.at(i).puntiScia.size() * sizeof(vec2), residui.at(i).puntiScia.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_RSC);
			glBufferData(GL_ARRAY_BUFFER, residui.at(i).coloreScia.size() * sizeof(vec4), residui.at(i).coloreScia.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, residui.at(i).puntiScia.size());
			glBindVertexArray(0);
		}

		if (sequenza_bomba > 0)
		{
			// disegna bomba
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_BMB);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_BMB);
			glBufferData(GL_ARRAY_BUFFER, disegnaBomba.size() * sizeof(vec2), disegnaBomba.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_BMBC);
			glBufferData(GL_ARRAY_BUFFER, coloreBomba.size() * sizeof(vec4), coloreBomba.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(10.0f * (float)sequenza_bomba);
			glDrawArrays(GL_LINE_STRIP, 0, disegnaBomba.size());
			glBindVertexArray(0);
		}

		// disegna statsBar
		Model = mat4(1.0);
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_SB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_SB);
		glBufferData(GL_ARRAY_BUFFER, disegnaStatsBar.size() * sizeof(vec2), disegnaStatsBar.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_SBC);
		glBufferData(GL_ARRAY_BUFFER, coloreStatsBar.size() * sizeof(vec4), coloreStatsBar.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glLineWidth(2.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, disegnaStatsBar.size());
		glBindVertexArray(0);

		// disegna testo statsBar
		for (int i = 0; i < textStatsBar.size(); i++)
		{
			Text text = textStatsBar.at(i);

			if (text.visible)
			{
				for (int j = 0; j < text.letters.size(); j++)
				{
					Letter let = text.letters.at(j);

					glm::mat4 modelMatrix = glm::mat4(1.0);
					modelMatrix = translate(modelMatrix, glm::vec3(text.pos.x, text.pos.y, 0.0f));
					modelMatrix = scale(modelMatrix, glm::vec3(text.scale, text.scale, 0.0f));
					glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(modelMatrix));

					glBindVertexArray(let.VAO);
					glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Geom);
					glBufferData(GL_ARRAY_BUFFER, let.points.size() * sizeof(vec2), let.points.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Col);
					glBufferData(GL_ARRAY_BUFFER, let.colors.size() * sizeof(vec4), let.colors.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(1);
					if (let.sizePoints > 0.0f)
						glPointSize(let.sizePoints);
					else glPointSize(1.0f);
					if (let.widthLines > 0.0f)
						glLineWidth(let.widthLines);
					else glLineWidth(1.0f);

					glDrawArrays(let.drawMode, 0, let.points.size());

					glBindVertexArray(0);
				}
			}
		}

		// disegna healthBar
		Model = mat4(1.0);
		glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
		glBindVertexArray(VAO_SB);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_SB);
		glBufferData(GL_ARRAY_BUFFER, disegnaHealthBar.size() * sizeof(vec2), disegnaHealthBar.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_SBC);
		glBufferData(GL_ARRAY_BUFFER, coloreHealthBar.size() * sizeof(vec4), coloreHealthBar.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glLineWidth(2.0f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, disegnaHealthBar.size());
		glBindVertexArray(0);

		if (pause)
		{

			// disegna background menu
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_M);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_M);
			glBufferData(GL_ARRAY_BUFFER, disegnaBackgroundMenu.size() * sizeof(vec2), disegnaBackgroundMenu.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_MC);
			glBufferData(GL_ARRAY_BUFFER, coloreBackgroundMenu.size() * sizeof(vec4), coloreBackgroundMenu.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, disegnaBackgroundMenu.size());
			glBindVertexArray(0);

			// disegna contorno menu
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_M);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_M);
			glBufferData(GL_ARRAY_BUFFER, disegnaContornoMenu.size() * sizeof(vec2), disegnaContornoMenu.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_MC);
			glBufferData(GL_ARRAY_BUFFER, coloreContornoMenu.size() * sizeof(vec4), coloreContornoMenu.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_LINES, 0, disegnaContornoMenu.size());
			glBindVertexArray(0);

			// disegna box menu
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_M);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_M);
			glBufferData(GL_ARRAY_BUFFER, disegnaBox.size() * sizeof(vec2), disegnaBox.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_MC);
			glBufferData(GL_ARRAY_BUFFER, coloreBox.size() * sizeof(vec4), coloreBox.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, disegnaBox.size());
			glBindVertexArray(0);

			// disegna lucchetti menu
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_M);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_M);
			glBufferData(GL_ARRAY_BUFFER, disegnaLucchetto.size() * sizeof(vec2), disegnaLucchetto.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_MC);
			glBufferData(GL_ARRAY_BUFFER, coloreLucchetto.size() * sizeof(vec4), coloreLucchetto.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, disegnaLucchetto.size());
			glBindVertexArray(0);

			// disegna arco lucchetti menu
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_M);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_M);
			glBufferData(GL_ARRAY_BUFFER, disegnaArcoLucchetto.size() * sizeof(vec2), disegnaArcoLucchetto.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_MC);
			glBufferData(GL_ARRAY_BUFFER, coloreArcoLucchetto.size() * sizeof(vec4), coloreArcoLucchetto.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, disegnaArcoLucchetto.size());
			glBindVertexArray(0);
						
			// disegna triangoli hovered
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_H);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_H);
			glBufferData(GL_ARRAY_BUFFER, disegnaHover.size() * sizeof(vec2), disegnaHover.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_HC);
			glBufferData(GL_ARRAY_BUFFER, coloreHover.size() * sizeof(vec4), coloreHover.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, disegnaHover.size());
			glBindVertexArray(0);

			// disegna linee hovered
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_HL);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_HL);
			glBufferData(GL_ARRAY_BUFFER, disegnaHoverL.size() * sizeof(vec2), disegnaHoverL.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_HLC);
			glBufferData(GL_ARRAY_BUFFER, coloreHoverL.size() * sizeof(vec4), coloreHoverL.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_LINES, 0, disegnaHoverL.size());
			glBindVertexArray(0);

			// disegna testo menu
			for (int i = 0; i < textMenu.size(); i++)
			{
				Text text = textMenu.at(i);

				if (text.visible)
				{
					for (int j = 0; j < text.letters.size(); j++)
					{
						Letter let = text.letters.at(j);

						glm::mat4 modelMatrix = glm::mat4(1.0);
						modelMatrix = translate(modelMatrix, glm::vec3(text.pos.x, text.pos.y, 0.0f));
						modelMatrix = scale(modelMatrix, glm::vec3(text.scale, text.scale, 0.0f));
						glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(modelMatrix));

						glBindVertexArray(let.VAO);
						glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Geom);
						glBufferData(GL_ARRAY_BUFFER, let.points.size() * sizeof(vec2), let.points.data(), GL_STATIC_DRAW);
						glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
						glEnableVertexAttribArray(0);
						glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Col);
						glBufferData(GL_ARRAY_BUFFER, let.colors.size() * sizeof(vec4), let.colors.data(), GL_STATIC_DRAW);
						glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
						glEnableVertexAttribArray(1);
						if (let.sizePoints > 0.0f)
							glPointSize(let.sizePoints);
						else glPointSize(1.0f);
						if (let.widthLines > 0.0f)
							glLineWidth(let.widthLines);
						else glLineWidth(1.0f);

						glDrawArrays(let.drawMode, 0, let.points.size());

						glBindVertexArray(0);
					}
				}
			}

			for (int i = 0; i < quadratiPerks.size(); i++)
			{
				if (game.perksActive[i])
				{
					Model = mat4(1.0);
					glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
					glBindVertexArray(VAO_PS);
					glBindBuffer(GL_ARRAY_BUFFER, VBO_PS);
					glBufferData(GL_ARRAY_BUFFER, quadratiPerks.at(i).disegnaActivePerkSquare.size() * sizeof(vec2), quadratiPerks.at(i).disegnaActivePerkSquare.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, VBO_PSC);
					glBufferData(GL_ARRAY_BUFFER, quadratiPerks.at(i).coloreActivePerkSquare.size() * sizeof(vec4), quadratiPerks.at(i).coloreActivePerkSquare.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(1);
					glLineWidth(2.0f);
					glDrawArrays(GL_TRIANGLES, 0, quadratiPerks.at(i).disegnaActivePerkSquare.size());
					glBindVertexArray(0);
				}
			}

			// DISEGNA quadrati verdi sotto le perk attive - rgb (0.46, 0.86, 0.46)

			// disegna linee perk menu
			int x = width * 210 / 1000;
			int y = height * 420 / 1000;
			glm::mat4 modelMatrix = glm::mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(modelMatrix));
			glBindVertexArray(VAO_DP);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_DP);
			glBufferData(GL_ARRAY_BUFFER, disegnaPerks.size() * sizeof(vec2), disegnaPerks.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_DPC);
			glBufferData(GL_ARRAY_BUFFER, colorePerks.size() * sizeof(vec4), colorePerks.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_LINES, 0, disegnaPerks.size());
			glBindVertexArray(0);

			// disegna triangoli perk menu
			modelMatrix = glm::mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(modelMatrix));
			glBindVertexArray(VAO_DPT);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_DPT);
			glBufferData(GL_ARRAY_BUFFER, disegnaPerksT.size() * sizeof(vec2), disegnaPerksT.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_DPTC);
			glBufferData(GL_ARRAY_BUFFER, colorePerksT.size() * sizeof(vec4), colorePerksT.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, disegnaPerksT.size());
			glBindVertexArray(0);
		}

		// disegna sequenza gameOver
		if (gameOver)
		{

			if (textGameOver.visible)
			{
				for (int j = 0; j < textGameOver.letters.size(); j++)
				{
					Letter let = textGameOver.letters.at(j);

					glm::mat4 modelMatrix = glm::mat4(1.0);
					modelMatrix = translate(modelMatrix, glm::vec3(textGameOver.pos.x, textGameOver.pos.y, 0.0f));
					modelMatrix = scale(modelMatrix, glm::vec3(textGameOver.scale, textGameOver.scale, 0.0f));
					glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(modelMatrix));

					glBindVertexArray(let.VAO);
					glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Geom);
					glBufferData(GL_ARRAY_BUFFER, let.points.size() * sizeof(vec2), let.points.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Col);
					glBufferData(GL_ARRAY_BUFFER, let.colors.size() * sizeof(vec4), let.colors.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
					glEnableVertexAttribArray(1);
					glLineWidth(5);

					glDrawArrays(let.drawMode, 0, let.points.size());

					glBindVertexArray(0);
				}
			}


			// disegna triangoli bottoni gameover
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_BGO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_BGO);
			glBufferData(GL_ARRAY_BUFFER, disegnaBottoniGO.size() * sizeof(vec2), disegnaBottoniGO.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_BGOC);
			glBufferData(GL_ARRAY_BUFFER, coloreBottoniGO.size() * sizeof(vec4), coloreBottoniGO.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_TRIANGLES, 0, disegnaBottoniGO.size());
			glBindVertexArray(0);


			// disegna contorni bottoni gameover
			Model = mat4(1.0);
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glBindVertexArray(VAO_CBGO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_BGO);
			glBufferData(GL_ARRAY_BUFFER, disegnaContornoBGO.size() * sizeof(vec2), disegnaContornoBGO.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_CBGOC);
			glBufferData(GL_ARRAY_BUFFER, coloreContornoBGO.size() * sizeof(vec4), coloreContornoBGO.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glLineWidth(2.0f);
			glDrawArrays(GL_LINES, 0, disegnaContornoBGO.size());
			glBindVertexArray(0);

			for (int i = 0; i < textPunteggio.size(); i++)
			{
				Text text = textPunteggio.at(i);

				if (text.visible)
				{
					for (int j = 0; j < text.letters.size(); j++)
					{
						Letter let = text.letters.at(j);

						glm::mat4 modelMatrix = glm::mat4(1.0);
						modelMatrix = translate(modelMatrix, glm::vec3(text.pos.x, text.pos.y, 0.0f));
						modelMatrix = scale(modelMatrix, glm::vec3(text.scale, text.scale, 0.0f));
						glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(modelMatrix));

						glBindVertexArray(let.VAO);
						glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Geom);
						glBufferData(GL_ARRAY_BUFFER, let.points.size() * sizeof(vec2), let.points.data(), GL_STATIC_DRAW);
						glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
						glEnableVertexAttribArray(0);
						glBindBuffer(GL_ARRAY_BUFFER, let.VBO_Col);
						glBufferData(GL_ARRAY_BUFFER, let.colors.size() * sizeof(vec4), let.colors.data(), GL_STATIC_DRAW);
						glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
						glEnableVertexAttribArray(1);
						glLineWidth(3);

						glDrawArrays(let.drawMode, 0, let.points.size());

						glBindVertexArray(0);
					}
				}
			}
		}
	}

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	initSounds(16);
	playMusic("neuron");

	srand((unsigned int)time(NULL));
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
	//Evento movimento mouse
	glutMotionFunc(mouseDragEvent);
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