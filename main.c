#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>

#define TIMER_ID 0
#define TIMER_INTERVAL 16

#define MAXCVOROVA 12


typedef struct loop{
  float y, vy;
	float r;
}Loop;

typedef struct cvor{
    float x, y;
}CvorZice;

Loop loop;

//Koristimo fiksan niz da ne bismo iznova kreirali cvorove
//kada jedan izadje sa ekrana samo ga postavimo iza poslednje kreiranog cvora
CvorZice cvorovi[MAXCVOROVA];
int poslednjiCvor = -1;
int poslednjePredjenCvor = -1;

float brzina = 0.015;
float minimalniR = 0.5;
float brzinaSmanjenja = 0.0001;
float jacinaSkoka = 0.04;
float razmak = 1;
const float G = 0.002;

float debljinaZice = 0.2;

int score = 0;
int highscore = 0;

float loopAnimacija = 0;
float brzinaAnimacije = 2;
int prvaIgra = 1;

static int animation_ongoing;   /* Fleg koji odredjuje da li je
                                 * animacija u toku. */


/* Deklaracije callback funkcija. */
static void on_keyboard(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_timer(int value);
static void on_display(void);

//Generisemo sledeci cvor tako sto ga postavljamo na x+razmak od poslednji cvora i dajemo mu random visinu
void GenerisiSledeciCvor();
void GenerisiPocetnuZicu();
void Resetuj();

void PostaviSvetlo() {
	float pos[] = { 1, 2, 0, 0 };
	float col[] = { 1, 1, 1, 1 };
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
}



int main(int argc, char **argv)
{
	/* Inicijalizuje se GLUT. */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	/* Kreira se prozor. */
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);

	/* Registruju se funkcije za obradu dogadjaja. */
	glutKeyboardFunc(on_keyboard);
	glutDisplayFunc(on_display);
	glutReshapeFunc(on_reshape);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	srand(time(NULL));

	Resetuj();
	/* Na pocetku je animacija neaktivna */
	animation_ongoing = 0;

	/* Obavlja se OpenGL inicijalizacija. */
	glClearColor(0, 0, 0, 0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);


    /* Ulazi se u glavnu petlju. */
	PostaviSvetlo();
	glutMainLoop();

	return 0;
}



static void on_keyboard(unsigned char key, int x, int y)
{
	switch (key) {
    case 27:
        /* Zavrsava se program. */
        exit(0);
        break;

		case 'r':
		case 'R':
			Resetuj();
			break;
		case ' ':
		//brzinaAnimacije *= -1;
			if (!animation_ongoing) {
				glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
				animation_ongoing = 1;
				if (!prvaIgra){
					Resetuj();
				}
				else{
					prvaIgra = 0;
				}
			}
			
			loop.vy = jacinaSkoka*(loop.r*0.5+0.5); //loop.r*0.5+0.5 da bi sporije opadala jacina skoka od velicine loop-a
			break;
    
	}
}


float visinaZice;

void GenerisiSledeciCvor(){
	float newy = ((float)rand()/RAND_MAX)*2-1;

	if (poslednjiCvor == -1){
		cvorovi[0].x = cvorovi[0].y = 0;
		poslednjiCvor = 0;
		return;
	}
	else {
		int sledeciCvor = (poslednjiCvor+1)<MAXCVOROVA?(poslednjiCvor+1):0;
		cvorovi[sledeciCvor] = cvorovi[poslednjiCvor];
		cvorovi[sledeciCvor].y = newy;
		cvorovi[sledeciCvor].x += razmak;
	}
	poslednjiCvor++;
	if (poslednjiCvor >= MAXCVOROVA)
		poslednjiCvor = 0;
}

void GenerisiPocetnuZicu(){
	for (int i=0; i<MAXCVOROVA; i++){
		GenerisiSledeciCvor();
	}

}

void Resetuj() {
	loop.r = 1;
	loop.y = loop.vy = 0;
	poslednjiCvor = -1;
	poslednjePredjenCvor = 0;
	GenerisiPocetnuZicu();
	score = 0;
}

void Izgubio() {
	if (highscore < score){
		highscore = score;
	}
	animation_ongoing = 0;
}


void DetekcijaSudara(){
	float x1, y1, x2, y2;
	int predjeniCvor;
	for (int i=0; i<MAXCVOROVA; i++){
		if (cvorovi[i].x <= 0 && cvorovi[(i+1)%MAXCVOROVA].x >= 0){
			x1 = cvorovi[i].x;
			y1 = cvorovi[i].y;
			x2 = cvorovi[(i+1)%MAXCVOROVA].x;
			y2 = cvorovi[(i+1)%MAXCVOROVA].y;
			predjeniCvor = i;
			break;
		}
	}
	
	if (predjeniCvor != poslednjePredjenCvor){
		poslednjePredjenCvor = predjeniCvor;
		score++;
	}
	
	//y-y1 = ((y2-y1)/(x2-x1))*(x-x1);
	//loop je uvek na x = 0 
	//y = ((y2-y1)/(x2-x1))*(-x1)+y1
	visinaZice = ((y2-y1)/(x2-x1))*(-x1)+y1;
	
	float dy = visinaZice-loop.y;
	if (fabs(dy) > loop.r - debljinaZice/2){
		Izgubio();
	}
}



static void on_reshape(int width, int height){
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60,  (float)width/height, 0.01 ,  1000);
	
}

void CrtajLoop(){
	glPushMatrix();
	glScalef(1, loop.r, loop.r);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glBegin(GL_TRIANGLE_STRIP);
	glRotatef(loopAnimacija, 1, 0, 0);
	for (float fi=0; fi<=M_PI*2+0.01; fi+=M_PI/20){
		glColor3f(fi<M_PI?1:0, 0, fi<M_PI?0:1);	
		glNormal3f(0, sin(fi), cos(fi));
		glVertex3f(-0.1, sin(fi), cos(fi));
		glVertex3f(+0.1, sin(fi), cos(fi));
		//glutSolidCube(0.1);
	}
	glEnd();
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glPopMatrix();
}

void CrtajZicu(){
	for (int i=poslednjiCvor+1; i<MAXCVOROVA+poslednjiCvor-1; i++){
		glPushMatrix();
		float x1 = cvorovi[i%MAXCVOROVA].x;
		float x2 = cvorovi[(i+1)%MAXCVOROVA].x;
		float y1 = cvorovi[i%MAXCVOROVA].y;
		float y2 = cvorovi[(i+1)%MAXCVOROVA].y;
		float tmpx = (x1 + x2)/2;
		float tmpy = (y1 + y2)/2;

		//Efekat promene boje/stvaranja cvora
		float vidljivost = 1.5;
		float color = vidljivost-(fabs(tmpx)-vidljivost/2);
		color = color<0?0:color;
		color /= vidljivost;
		color *= color;

		glColor3f(color, color, color);

		glBegin(GL_TRIANGLE_STRIP);
		for (float fi=0; fi<=M_PI*2+0.1; fi+=M_PI/10){
			float y = sin(fi)*(debljinaZice/2);
			float z = cos(fi)*(debljinaZice/2);
			
			glNormal3f(0, y, z);
			glVertex3f(x1, y+y1, z);
			glVertex3f(x2, y+y2, z);
			
		}
		glEnd();

		glPopMatrix();
	}
}

void CrtajScore() {  
  char text[20];
	sprintf(text, "%d", score);
	char* c;
	
	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1);
	
  glRasterPos3f(0, loop.y+loop.r+0.2, 0);
  for (c=text; *c != '\0'; c++) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
  }
	
	sprintf(text, "Highscore: %d", highscore);
  glRasterPos3f(-1.5, 1.2, 0);
  for (c=text; *c != '\0'; c++) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
  }
	
	glEnable(GL_LIGHTING);
}

static void on_timer(int value)
{
	/*
	 * Proverava se da li callback dolazi od odgovarajuceg tajmera.
	 */
	if (value != TIMER_ID)
			return;
	
	//Kretanje loop-a
	loop.vy -= G; //dodaje se gravitacija na brzinu loop-a
	loop.y += loop.vy; //pomera se loop na osnovu brzine
	
	if (loop.r - brzinaSmanjenja >= minimalniR){
		loop.r -= brzinaSmanjenja;
	}
	else{
		loop.r = minimalniR;
	}
	//Pomeranje cvorova
	for (int i=0; i<MAXCVOROVA; i++){
		//Ovo je bilo ubrzanje ali posto se sada  smanjuje loop bilo bi pretesko da se desava i jedno i drugo
		cvorovi[i].x -= brzina; //(brzina+(float)score/6000); 
		if (cvorovi[i].x < -5){
			GenerisiSledeciCvor();
			break;
		}
	}
		
	DetekcijaSudara();
		
	loopAnimacija += brzinaAnimacije;
	/* Forsira se ponovno iscrtavanje prozora. */
	glutPostRedisplay();

	/* Po potrebi se ponovo postavlja tajmer. */
	if (animation_ongoing) {
			glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
	}
}

static void on_display(void)
{
	//glClearColor(backgroundComboColor, backgroundComboColor, backgroundComboColor, 0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(-2, 0, 3,
						0, 0, 0,
						0, 1, 0);



	float color[] = { 0, 0, 0};

	glPushMatrix();
	glTranslatef(0, loop.y, 0);
	glRotatef(loopAnimacija, 1, 0, 0);
	CrtajLoop();
	glPopMatrix();
	
	CrtajZicu();
	
	glPushMatrix();
	CrtajScore();
	glPopMatrix();
	
	PostaviSvetlo();
	
	glutSwapBuffers();
}
