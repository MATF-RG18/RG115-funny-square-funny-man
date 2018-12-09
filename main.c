#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>

#define TIMER_ID 0
#define TIMER_INTERVAL 20

int*** slika;

float animacija[9][9];

int popunjenost[9];
int trenutniLevel = 0;
int brojLevela = 2;

typedef enum b{
	CRVENA = 0,
	PLAVA = 1,
	ZELENA = 2,
	ZUTA = 3,
	CRNA = 4,
	BELA = 5
} Boja;
int brojBoja = 6;

int width, height;

float pozicijaIgraca = 0;
float blokX = -1, blokY = -1;
int bojaPadajuceg, bojaSledeceg;
int zivoti = 3;
float brzinaPadanja;
float osnovnaBrzina = 0.15;
Boja prvaBojaSlike, drugaBojaSlike;
float animationParametar;
static int animation_ongoing;


/* Deklaracije callback funkcija. */
static void on_timer(int value);
static void on_display(void);
static void on_keyboard(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_mouse_moved(int x, int y);

void restartujIgru();
void generisiNoviBlok();
void pomeriIgraca(int x);
int proveriPobedu();
void postaviLevel();
void animacijaSlike();
void postaviBoju(int b, float i);
void crtajSrce();
void ucitajLevele();

int main(int argc, char **argv)
{
    /* Inicijalizuje se GLUT. */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

	/* Kreira se prozor. */
	glutInitWindowSize(800, 600);
	glutCreateWindow(argv[0]);

	/* Registruju se funkcije za obradu dogadjaja. */
	glutKeyboardFunc(on_keyboard);
	glutDisplayFunc(on_display);
	glutReshapeFunc(on_reshape);

  /* Na pocetku je animacija neaktivna */
  animation_ongoing = 1;
  /* Obavlja se OpenGL inicijalizacija. */
  glClearColor(0.3, 0.3, 0.3, 0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	GLfloat lightPosition[] = { -1, -1, 1, 1};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	/* Ulazi se u glavnu petlju. */
	ucitajLevele();
	restartujIgru();

	on_timer(TIMER_ID);
	glutMainLoop();

	return 0;
}

void ucitajLevele(){
	FILE* f = fopen("levels.txt", "r");
	int n;
	fscanf(f, "%d", &n);
	brojLevela = n;
	slika = malloc(n*sizeof(int**));
	//Alokacija
	for (int i=0; i<n; i++){
		slika[i] = malloc(9*sizeof(int*));
		for (int j=0; j<9; j++){
			slika[i][j] = malloc(9*sizeof(int));
		}
	}

	//Ucitavanje

	for (int i=0; i<n; i++){
		for (int j=8; j>=0; j--){
			for (int k=0; k<9; k++){
				fscanf(f, "%d", &slika[i][j][k]);
			}
		}
	}
}

static void on_keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 27:
			/* Zavrsava se program. */
			exit(0);
			break;
		case 'a':
		case 'A':
			/* Zavrsava se program. */
			pomeriIgraca(-1);
			break;
		case 'd':
		case 'D':
			/* Zavrsava se program. */
			pomeriIgraca(1);
			break;
	}
}

void pomeriIgraca(int x) {
	pozicijaIgraca += x;
	if (pozicijaIgraca < 0)
		pozicijaIgraca = 0;
	if (pozicijaIgraca > 8)
		pozicijaIgraca = 8;
}

void restartujIgru(){
	srand(time(NULL));
	//Biraju se random boje koje ce se koristiti tokom trenutne partije
	prvaBojaSlike = rand()%brojBoja;
	drugaBojaSlike = prvaBojaSlike;
	while(drugaBojaSlike == prvaBojaSlike){
		drugaBojaSlike = rand()%brojBoja;
	}

	zivoti = 3;
	trenutniLevel = 0;
	//Vraca brzinu padanja na pocetnu
	brzinaPadanja = osnovnaBrzina;

	postaviLevel();

	pozicijaIgraca = 4;
	generisiNoviBlok();
}

int proveriPobedu(){
	for (int i=0; i<9; i++){
		if (popunjenost[i] != 9)
			return 0;
	}
	return 1;
}

void generisiNoviBlok(){
	//Generise blok na random x mestu gde nije napunjena kolona do kraja
	blokX = rand() % 9;
	while (popunjenost[(int)blokX] == 9){
		blokX = rand() % 9;
	}
	blokY = 10;
	bojaPadajuceg = rand()%2;
	//Dohvatamo boju sledeceg polja koji se nalazi u istoj koloni kao i ovaj sto se generisao
	int x = (int)blokX;
	int y = popunjenost[(int) blokX];
	bojaSledeceg = slika[trenutniLevel][y][x];
	animationParametar = 0;
}

void pomeriBlok(){
	//Spustamo blok na dole za brzinaPadanja
	//brzinaPadanja se povecava u on_timer
	blokY -= brzinaPadanja;

	if (blokY < -2){
		//Slucaj kada padne ispod igraca
		if (bojaPadajuceg == bojaSledeceg){
			//Ako je propustio boju koju je trebalo da pokupi
			zivoti--;
			if (zivoti == 0)
				restartujIgru();
		}
		generisiNoviBlok();
	}
	else if (blokY < 0 && blokY > -1 && blokX == pozicijaIgraca){
		//Slucaj kada dodiruje igraca
		if (bojaPadajuceg == bojaSledeceg) {
			//Ako je uhvatio boju koju je trebalo da pokupi
			popunjenost[(int)blokX]++;
			if (proveriPobedu()){
				return;
			}
			generisiNoviBlok();
		}
		else{
			//Ako je uhvatio boju koju nije trebalo da pokupi
			zivoti--;
			if (zivoti == 0){
				restartujIgru();
			}
			generisiNoviBlok();
		}
	}
}

void animacijaSlike(){
	//Prolazi kroz sve blokove i povecava im parametar animacije koji se koristi za
	//animaciju kretanja na gore kada se pokupi korektan blok
	for (int i=0; i<9; i++){
		for (int j=0; j<9; j++){
			if (popunjenost[j] > i && animacija[i][j] < 1){
				animacija[i][j] += 0.1;
			}
		}
	}
}

void postaviLevel(){
	for (int i=0; i<9; i++){
		for (int j=0; j<9; j++){
			animacija[i][j] = 0;
		}
	}
	for (int i=0; i<9; i++){
		popunjenost[i] = 0;
	}
}

static void on_timer(int value)
{
    /*
     * Proverava se da li callback dolazi od odgovarajuceg tajmera.
     */
    if (value != TIMER_ID)
        return;

    /* Forsira se ponovno iscrtavanje prozora. */
    glutPostRedisplay();

		//Ako se zavrsila igra ne pomeraj vise blok koji pada
		if (!proveriPobedu()){
			pomeriBlok();
		}
		else{
			if (trenutniLevel+1 < brojLevela){
				trenutniLevel++;
				postaviLevel();
				generisiNoviBlok();
			}
		}
		animacijaSlike();

		animationParametar += 0.1;
		brzinaPadanja += 0.00001;
    /* Po potrebi se ponovo postavlja tajmer. */
    if (animation_ongoing) {
        glutTimerFunc(TIMER_INTERVAL, on_timer, TIMER_ID);
    }
}

void postaviBoju(int b, float i){
	//Za svaku boju postavljamo rgb kombinaciju koja ce se koristiti
	//i se koristi za efekat treperenja
	Boja boja = b ? prvaBojaSlike : drugaBojaSlike;
	switch (boja) {
		case CRVENA:
			glColor3f(1-i, i, i);
			break;
		case PLAVA:
			glColor3f(i, i, 1-i);
			break;
		case ZELENA:
			glColor3f(i, 1-i, i);
			break;
		case ZUTA:
			glColor3f(1-i, 1-i, i);
			break;
		case CRNA:
			glColor3f(0.13+i/4, 0.13+i/4, 0.13+i/4);
			break;
		case BELA:
			glColor3f(1-i, 1-i, 1-i);
			break;
	}
}

static void on_reshape(int w, int h)
{
    /* Pamte se sirina i visina prozora. */
    width = w;
    height = h;

    /* Podesava se viewport. */
    glViewport(0, 0, width, height);

    /* Podesava se projekcija. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70, (float) width / height, 0.1, 100);
}

void crtajSrce(){
	glPushMatrix();
	glutSolidSphere(0.2, 10, 10);
	glTranslatef(0.2, 0, 0);
	glutSolidSphere(0.2, 10, 10);
	glTranslatef(-0.1, -0.2, 0);
	glRotatef(45, 0, 0, 1);
	glutSolidCube(0.2);
	glPopMatrix();
}

static void on_display(void)
{
	glEnable(GL_LIGHTING);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(4, 4, 10,
						4, 4, 0,
						0, 1, 0);

	//Crta blokove slike
	for (int i=0; i<9; i++){
		for (int j=0; j<9; j++){
			//Prekace iscrtavanje bloka slike ako se jos nije popunio
			if (i >= popunjenost[j]){
				continue;
			}

			glPushMatrix();
			postaviBoju(slika[trenutniLevel][i][j], 0);
			glTranslatef(j, i, slika[trenutniLevel][i][j]*0.3);
			//Animacija podizanja u zavisnosti od animacija[][]
			glTranslatef(0, -1+animacija[i][j], 0);
			glutSolidCube(1);

			glPopMatrix();
		}
	}


	//Crta blok koji hvata
	glPushMatrix();
	float intenzitet = 0;
	//Animacija treperenja
	intenzitet = fabs(cos(animationParametar)*0.2);
	postaviBoju(bojaSledeceg, intenzitet);
	glTranslatef(pozicijaIgraca, -1, 1);
	//glScalef(1, 0.5, 1);
	glutSolidCube(1);
	glPopMatrix();

	//Crta blok koji pada
	if (!proveriPobedu()){
		//Dodatan blok oko ovog sto se krece zbog bolje vidljivosti
		glPushMatrix();
		glColor3f(0, 0, 0);
		glTranslatef(blokX, blokY, 1);
		glutSolidCube(1.1);
		postaviBoju(bojaPadajuceg, 0);
		glScalef(1, 1, 1.2);
		glutSolidCube(0.95);
		glPopMatrix();
	}
	else{
		//Ispisuje "you win" kada predjes igru
		char* winText = "You win!";
		glColor3f(0, 0, 0);
		glRasterPos3f(3.5, 3.2, 4);
		int n = strlen(winText);
		for (int i = 0; i < n; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, winText[i]);
		}
		glColor3f(1, 1, 1);
		glRasterPos3f(3.5, 3.2, 4.2);
		for (int i = 0; i < n; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, winText[i]);
		}
	}

	//Crta srca
	//Srca menjaju boju sama od sebe zbog prethodnog postavljanja boje
	//Ali mi ovako izgleda bas kul :D
	glPushMatrix();
	glTranslatef(0, 10, 0);
	for (int i=0; i< zivoti; i++){
		glPushMatrix();
			glTranslatef(i, 0, 0);
			crtajSrce();
		glPopMatrix();
	}

	glPopMatrix();

	//Crta resetku
	glDisable(GL_LIGHTING);
	postaviBoju(0, 0);
	glPushMatrix();
	glTranslatef(4, 4, 0);
	glScalef(9, 9, 1);
	glutWireCube(1);
	glPopMatrix();

	for (int i=-10; i<20; i++){
		for (int j=-10; j<20; j++){
			glColor3f(0.4, 0.4, 0.4);
			glPushMatrix();
			glTranslatef(j, i, 0);
			if (i>=0 && i < 9 && j>=0 && j<9){
				glTranslatef(0, 0, slika[trenutniLevel][i][j]*0.3);
				postaviBoju(slika[trenutniLevel][i][j], 0);
			}
			glutWireCube(1);

			glPopMatrix();
		}
	}


	glutSwapBuffers();
}
