#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <GL/glut.h>
#include <math.h>
#include <string.h>

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

typedef enum k{
	NORMALNA,
	PRELAZAK,
	KRAJ
} StanjeKamere;

StanjeKamere stanjeKamere = NORMALNA;
float animacijaKamere = 0;

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
int komboBrojac = 0;
int potrebnoZaNovZivot = 20;
int potrebnoZaScoreKombo = 10;
int poeni = 0;
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
void crtajBlokove();
void crtajMrezu();
void crtajTekst(char* text, float x, float y);
void crtajZivote();
void ucitajLevele();
void predjiLevel();
void dodajZivot();

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
				fscanf(f, "%d", &slika[n-i-1][j][k]);
			}
		}
	}
}

void predjiLevel(){
	for (int i=0; i<9; i++){
		popunjenost[i] = 9;
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
			/* kretanje ulevo */
			pomeriIgraca(-1);
			break;
		case 'd':
		case 'D':
			/* kretanje udesno */
		pomeriIgraca(1);
			break;
		case 'r':
		case 'R':
			/* restartovanje igre . */
			restartujIgru();
			break;
		case ' ':
			/* Cheat za prelazak levela. */
			predjiLevel();
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

	stanjeKamere = NORMALNA;
	animacijaKamere = 0;

	poeni = 0;
	zivoti = 3;
	komboBrojac = 0;
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

void dodajZivot(){
	if (komboBrojac != 0 && komboBrojac % potrebnoZaNovZivot == 0){
		zivoti++;
	}
}

void dodajPoene(){
	int d = pow(2, (komboBrojac-1)/potrebnoZaScoreKombo);
	poeni += d*10;
}

void generisiNoviBlok(){
	//Generise blok na random x mestu gde nije napunjena kolona do kraja
	blokX = rand() % 9;
	while (popunjenost[(int)blokX] == 9){
		blokX = rand() % 9;
	}
	blokY = 10;
	//Dohvatamo boju sledeceg polja koji se nalazi u istoj koloni kao i ovaj sto se generisao
	int x = (int)blokX;
	int y = popunjenost[(int) blokX];
	bojaSledeceg = slika[trenutniLevel][y][x];
	bojaPadajuceg = bojaSledeceg;
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
			komboBrojac = 0;
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
			komboBrojac++;
			dodajPoene();
			dodajZivot();
			if (proveriPobedu()){
				return;
			}
			generisiNoviBlok();
		}
		else{
			//Ako je uhvatio boju koju nije trebalo da pokupi
			zivoti--;
			komboBrojac = 0;
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

		if (stanjeKamere == NORMALNA){
			//Ako se zavrsila igra ne pomeraj vise blok koji pada
			if (!proveriPobedu()){
				pomeriBlok();
			}
			else{
				if (trenutniLevel+1 < brojLevela){
					trenutniLevel++;
					postaviLevel();
					generisiNoviBlok();
					stanjeKamere = PRELAZAK;
				}
				else{
					stanjeKamere = KRAJ;
				}
			}
			animacijaSlike();

			animationParametar += 0.1;
			brzinaPadanja += 0.000004;
		}
		else{
			animacijaKamere += 0.02;
			if (animacijaKamere > 1){
				animacijaKamere = 1;
				if (stanjeKamere == PRELAZAK){
					animacijaKamere = 0;
					stanjeKamere = NORMALNA;
				}
			}
		}

		/* Forsira se ponovno iscrtavanje prozora. */
		glutPostRedisplay();
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

	//U zavisnosti od stanjeKamere pomeramo menjamo njenu poziciju
	//Ako je NORMALNA to znaci da se igra trenutni level
	//Ako je PRELAZAK to znaci da smo zavrsili level i kamera treba da se pomeri na sledeci
	//Ako je KRAJ to znaci da smo zavrsili sve levele i kamera treba da se udalji da bi se videla cela slika

	if (stanjeKamere == NORMALNA) {
			gluLookAt(4, 4, 10,
				4, 4, 0,
				0, 1, 0);
	}
	else if (stanjeKamere == PRELAZAK){
		//Pomeramo kameru od prethodnog ka sledecem levelu
		float posY = -4+animacijaKamere*8;
		gluLookAt(4, posY, 10,
			4, posY, 0,
			0, 1, 0);
	}

	else{
		//Pozicioniramo kameru tako da ide ka sredini cele slike po y
		float posY = 4-((brojLevela-1)/2.0)*8*animacijaKamere;
		//Pozicioniramo kameru tako da se udaljava da bi se videla cela slika
		float posZ = 10+15*animacijaKamere;

		gluLookAt(4, posY , posZ,
			4, posY, 0,
			0, 1, 0);
	}

	crtajBlokove();

	//Crta blok koji hvata
	glPushMatrix();
	float intenzitet = 0;
	//Animacija treperenja
	intenzitet = fabs(cos(animationParametar)*0.2);
	postaviBoju(bojaSledeceg, intenzitet);
	glTranslatef(pozicijaIgraca, -1, 1);
	//glScalef(1, 0.5, 1);
	if (stanjeKamere != KRAJ)
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
		char winText[50];
		sprintf(winText, "You won!\nYour score is %d", poeni);
		glColor3f(0, 0, 0);
		glRasterPos3f(0, 0, 4);
		int n = strlen(winText);
		for (int i = 0; i < n; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, winText[i]);
		}
		glColor3f(1, 1, 1);
		glRasterPos3f(0, 0, 4.2);
		for (int i = 0; i < n; i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, winText[i]);
		}
	}

	//Crta score
	glColor3f(1, 1, 1);
	char score[20];
	sprintf(score, "%d", poeni);
	crtajTekst(score, 8, 10);


	glColor3f(1, 0, 0);
	int kombo = pow(2, komboBrojac/potrebnoZaScoreKombo);
	sprintf(score, "X%d", kombo);
	crtajTekst(score, 6, 10);


	//Crta srca
	//Srca menjaju boju sama od sebe zbog prethodnog postavljanja boje
	//Ali mi ovako izgleda bas kul :D
	crtajZivote();

	crtajMrezu();


	glutSwapBuffers();
}

void crtajTekst(char* text, float x, float y){
	glRasterPos3f(x, y, 0);
	int n = strlen(text);
	for (int i = 0; i < n; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
	}
}

void crtajZivote(){
	glPushMatrix();
	glTranslatef(0, 10, 0);
	for (int i=0; i< zivoti; i++){
		glPushMatrix();
			glTranslatef(i, 0, 0);
			crtajSrce();
		glPopMatrix();
	}
	glPopMatrix();

	glColor3f(0, 0, 0);

	float procenat = (float)(komboBrojac%potrebnoZaNovZivot + 1)/potrebnoZaNovZivot;
	float visina = procenat*9;
	glPushMatrix();
	glTranslatef(-2, visina/2-0.5, 0);
	glScalef(0.5, visina, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-2, 4, 0);
	glScalef(0.5, 9, 0.5);
	glutWireCube(1);
	glPopMatrix();


}

void crtajBlokove(){
	//Crta blokove slike
	for (int k=0; k<=trenutniLevel; k++){
		for (int i=0; i<9; i++){
			for (int j=0; j<9; j++){
				//Crtamo sve blokove na svim prethodnim levelima osim na trenutnom
				if (trenutniLevel == k){
					//Na trenutnom levelu crtamo samo one koji su popunjeni
					if (i >= popunjenost[j]){
						continue;
					}
				}

				glPushMatrix();
				postaviBoju(slika[k][i][j], 0);
				//Blokove sa prethodnih levela crtamo ispod trenutnog
 				//(trenutniLevel-k)*8 je pometaj bloka u odnosu na trenutni level
				glTranslatef(j, i - (trenutniLevel-k)*8, slika[k][i][j]*0.3);
				//Animacija podizanja u zavisnosti od animacija[][]
				glTranslatef(0, -1+animacija[i][j], 0);
				glutSolidCube(1);

				glPopMatrix();
			}
		}
	}
}

void crtajMrezu(){
	//Crta resetku
	glDisable(GL_LIGHTING);
	postaviBoju(0, 0);
	glPushMatrix();
	glTranslatef(4, 4, 0);
	glScalef(9, 9, 1);
	glutWireCube(1);
	glPopMatrix();

	for (int i=-30; i<40; i++){
		for (int j=-30; j<40; j++){
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
}
