#include <stdio.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

struct Vector { float x; float y; } typedef Vector;

// Vector constructor
Vector* NewVector(float x, float y) {
  Vector* vector = (Vector*)malloc(sizeof(Vector));

  vector -> x = x;
  vector -> y = y;

  return vector;
}

// Multiplies a vector for a scalar.
void Mult(Vector* v1, float number) {
   v1 -> x *= number;
   v1 -> y *= number;
}

// Sums a vector to another.
void Sum(Vector* v1, Vector* v2) {
   v1 -> x += v2 -> x;
   v1 -> y += v2 -> y;
}

int GetSignal(int num) {
  if (num == 0) return 0;
  return num > 0 ? 1 : -1;
}

// Simulation variables
#define numParticles 100

#define width  160 
#define height 44 

float deltaTime =  1. / 60;
float collDamping = .2;

// Particle vectors
Vector* positions[numParticles];
Vector* velocities[numParticles];

// Buffer for drawing
char buffer[width * height];

// Characters drawn
int densityASCII[] = { '@', '%', '8', '&', '#', '!', '*', ':', '.' };
int backgroundASCII = ' ';

float initialDst = 1;

void CheckBoundCollisions() {
  for (int i = 0; i < numParticles; i++) {
    if (positions[i] -> x >= (float) width / 2) {
      positions[i]  -> x = (float) width / 2 * GetSignal(positions[i] -> x);
      velocities[i] -> x *= -(1 - collDamping);
    }
    
    if (positions[i] -> y >= (float) height / 2) {
      positions[i]  -> y = (float) height / 2 * GetSignal(positions[i] -> y);
      velocities[i] -> y *= -(1 - collDamping);
    }
  }
}

void Start() {
  printf("\x1b[2J");

  // Displaying particles in grid
  int side = sqrt(numParticles);
  for (int i = 0; i < numParticles; i++) {
    Vector* initialPoss = NewVector(i % side, i /  side);
    Vector* initialVels = NewVector(0, 0);

    Mult(initialPoss, initialDst);
    Sum(initialPoss, &(Vector){ (float) (initialDst - side) / 2 , (float) (initialDst - side) / 2 });

    positions[i] = initialPoss;
    velocities[i] = initialVels;
  }
}

void Update() {
  memset(buffer, backgroundASCII, width * height);

  for (int i = 0; i < numParticles; i++) {
    Sum(velocities[i], &(Vector){0, 200 * deltaTime} ); // Gravity acceleration
   
    Vector* posVar = velocities[i]; 
    Mult(posVar, (double) deltaTime); 
    Sum(positions[i], posVar); 

    int x = (int) positions[i] -> x * 2 + (float) width  / 2;
    int y = (int) positions[i] -> y     + (float) height / 2; 

    int bufferIdx = x + y * width;
    if (bufferIdx < 0 || bufferIdx > width * height) continue;

    //if (i == 0) printf("%f\n", velocities[i] -> y);
    buffer[bufferIdx] = densityASCII[0];
  }

  CheckBoundCollisions();

  // Drawing buffer
  printf("\x1b[H");
  for (int k = 0; k < width * height; k++) putchar(k % width ? buffer[k] : 10);
}

int main() {
  Start();

  while (true) Update();

  return 0;
}

