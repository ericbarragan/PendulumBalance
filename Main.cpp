#include <iostream>
#include <math.h>
#include <time.h>
#include <chrono>
#include <thread>
#include <GL/glut.h>

/* Basado en La Serie Pendulo0x.c  Pretende orientar esta soluciona Objetos
 *  para permitir mas reusabilidad y facilitar aumentar la abstraccion
 */

 //#define ARCHIVO "PesosPendulo4.pes"
#define PI 3.14159265359
#define e  2.71828182845904523536
#define Rng 1/8

 // Variables Control de Fraccion de Tiempo
struct timespec ts;	// Estructura para Leer el Tiempo en nanosegundos
int FT = 50;		// Fraccion de Tiempo por segundo
//~ int CF = 5;			// Ciclos de Tiempo en que se aplica la Fuerza

// Parametros Iniciales
int Encendido = 1;		// Perceptron Trabajando
int Activo = 0;			// Evalua y Ajusta, aun fuera de entrenamiento, requerido para Entrenar
int Entrenamiento = 0;	// Genera situaciones para Entrenamiento
int Cargar = 1;			// Toma los Pesos del Archivo
int Guardar = 0;		// Guarda a Archivo Pesos Entrenados
int Ciclos = 2000;		// Ciclos de Entrenamiento
int Sigmoide = 1;		// En el calculo usa la funcion
int running = 0;		// 1 Tiempo en que Simula, 0 Tiempo en que Evalua, Ajusta, Entrena
int Itr = 0;			// Numero de Iteraciones ejecutadas
int Pesos = 4;		// Numero de Pesos a Trabajar
float Apr = 0.1f;		// Fraccion de Magnitud de Error para Aprender
float Error = 0.0;		// Error
double Pes[4];		// Arreglo de Pesos

// Inicializa Variables
int E = 100;			// Escala para Grafica
float G = 9.8f;		// Gravedad
float Fuerza = 0.0;	// Fuerza
float t_ini = 1.0;	// Angulo con el que inicia la Simulacion
float v_ini = 0.1f;	// Velocidad con la que inicia la Simulacion

// Propiedades Carro
float M = 1.0;	// Masa
// float D = pow(M, 0.33333333);	// Dimension calculada por Masa, estetico
float D = 1.0;	// Dimension
float b = 0.01f;	// friccion
float X = 0.0f;	// Posicion
float V = 0.0f;	// Velocidad
float A = 0.0f;	// Aceleracion
float F = 0.0f;	// Fuerza
// Propiedades Bola
float m = 1.0;	// masa
//float d = pow(m, 0.33333333);	// Dimension calculada por masa, estetico
float d = 1.0f;	// dimension
float l = 1.0f;	// longitud
float t = 0.01f;	// angulo
float x = 0.0f;	// x de Bola
float y = 0.0f;	// y de Bola
float v = 0.0f;	// velocidad
float a = 0.0f;	// aceleracion
float It = 0.0f;	// angulo inicial
float Iv = 0.0f;	// velocidad inicial


void bola() {
	int L = 16;
	float x, y, rad = 10.0, angle, arc = 2.0 * PI / L;
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i <= L; i++) {
		angle = arc * i;
		x = cos(angle) * rad;
		y = sin(angle) * rad;
		glVertex2d(x, y);
	}
	glEnd();
}

void rect() {
	float l = 25.0, a = 30.0;
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex2f(-l, 0);
	glVertex2f(l, 0);
	glVertex2f(l, -a);
	glVertex2f(-l, -a);
	glEnd();
}

void bara(float x1, float y1, float x2, float y2) {
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

// Calculos
void Actualiza() {
	float B, S, C;

	// Direccion de Friccion
	if (V > 0)
		B = b;
	else if (V < 0)
		B = -b;
	else
		B = 0;

	// Simulacion
	S = sin(t);
	C = cos(t);

	/*
	A	= (	+ F
			- B * M * G
			- B * C * C * m * G
			+ B * C * m * v*v / l
			+ S * C * m * G			// En prueba
			- S * m * v*v / l
		  )
		  /	( M
			+ S * S * m
			+ B * C * S * m
			);
	*/

	A = (+F
		- B * M * G
		- B * C * C * m * G
		+ B * C * m * v * v / l
		+ S * C * m * G			// En prueba
		- S * m * v * v / l
		)
		/ (M
			+ S * S * m
			+ B * C * S * m
			);

	//~ a = S * G - C * A;
	a = S * G + C * A;

	printf("F:%f\n\n", S * C * m * G);
	if (t > PI - 0.1) {
		printf("F:%f  A:%f  G:%f  M:%f  V:%f  B:%f  BF:%f VF:%f \n\n", F, A, G, M, V, B, -B * ((M * G)), A / FT);
	}
	V += A / (2 * FT);
	v += a / (2 * FT);
	// Con conversion 
	X -= V * E / FT;
	t += v / FT;
	F = 0;

	// De vuelta en la pantalla
	if (X > 400) X = -400;
	if (X < -400) X = 400;
	// Mantiene el angulo entre -2pi y 2pi
	if (t > PI) t -= 2 * PI;
	if (t < -PI) t += 2 * PI;
	// Pone límite a las velocidades
	if (V > 10) V = 10;
	if (V < -10) V = -10;
	if (v > 2 * PI) v = 2 * PI;
	if (v < -2 * PI) v = -2 * PI;

	// Posicion Bola, respecto a carro
	x = X + sin(t) * l * E;
	y = cos(t) * l * E;
}

static void Timer(int value) {
	timespec_get(&ts, TIME_UTC);
	ts.tv_sec = 0;
	ts.tv_nsec = (long)((1000000000 / FT) - (ts.tv_nsec) % (1000000000 / FT));
	//	nanosleep(&ts, NULL);
	std::this_thread::sleep_for(std::chrono::milliseconds(ts.tv_nsec / 1000000));

	Actualiza();

	glFlush();
	glutPostRedisplay();
	glutTimerFunc(1, Timer, 0);
}

/*
void Timer(){

	while (1){
		timespec_get(&ts, TIME_UTC);

		//~ printf("1 Current time: %06ld UTC\n", ts.tv_nsec/1000);

		//~ if (CF>0) printf("%f \n\n", F);
		//~ CF--;

		ts.tv_sec = 0;
		ts.tv_nsec = (long)((1000000000 / FT)  -  (ts.tv_nsec)  %  (1000000000 / FT));
		nanosleep(&ts, NULL);

		//~ Control();
		Actualiza();

		glFlush ();
		glutPostRedisplay();
		//~ glutTimerFunc(1/FT,Timer,0);
	}
}
*/


void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 0.0);

	glPushMatrix();
	glTranslatef(X, 0, 0);
	rect();
	glPopMatrix();

	glPushMatrix();
	bara(X, 0, x, y);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(x, y, 0);
	bola();
	glPopMatrix();

	glutSwapBuffers();
}

void NormalKeys(unsigned char key, int x, int y) {
	if (key == 27) { // esc
		exit(0);
	}
	else if (key == 54) { // 6
		X += 2.0;
	}
	else if (key == 52) { // 4
		X -= 2.0;
	}
	else if (key == 56) { // 8
		y += 0.1;
	}
	else if (key == 50) { // 2
		y -= 0.1;
	}
	else if (key == 48) { // 0
		printf("0 \n\n");
		Encendido = 0;
		Activo = 0;
		Entrenamiento = 0;
		running = 0;
	}
	else if (key == 53) { // 5
		printf("5 \n\n");
		Encendido = 1;
	}
	printf("%i \n\n", Encendido);
}

void Specialkeys(int key, int A, int B) {
	switch (key) {
	case GLUT_KEY_HOME:
		exit(0);
		break;
	case GLUT_KEY_RIGHT:
		F -= 200;
		break;
	case GLUT_KEY_LEFT:
		F += 200;
		break;
	case GLUT_KEY_UP:
		t += 0.1;
		break;
	case GLUT_KEY_DOWN:
		t -= 0.1;
		break;
	default:
		break;
	}
}

int main(int argc, char** argv) {
	//~ CargaPesos(Cargar);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 400);
	glutInitWindowPosition(250, 150);
	glutCreateWindow("Carro de Prueba 2D");

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-400.0, 400.0, -250.0, 150.0, 0, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0, 1.0, 1.0, 0.0);

	Timer(0);
	//~ glutTimerFunc(500, display, 0);
	glutDisplayFunc(display);
	glutSpecialFunc(Specialkeys);
	glutKeyboardFunc(NormalKeys);
	glutMainLoop();
}
