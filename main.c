#include <unistd.h>
#include <stdio.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

struct Vector { double x; double y; } typedef Vector;

// Vector constructor
Vector* NewVector(double x, double y) {
  Vector* vector = (Vector*)malloc(sizeof(Vector));

  vector -> x = x;
  vector -> y = y;

  return vector;
}

// Returns memory copy of another vector
Vector* CopyVector(Vector* vector) {
  return NewVector(vector -> x, vector -> y);  
}

// Calculates magnitude squared of a vector
float MagSqrd(Vector* vec) {
  return pow(vec -> x, 2) + pow(vec -> y, 2);
}

// Calculates Magnitude of a vector
float Mag(Vector* vec) {
  return sqrt(MagSqrd(vec));
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

Vector* RandomDir() {
  Vector* vector = NewVector((float) rand() / RAND_MAX, (float) rand() / RAND_MAX);
  Mult(vector, 1 / Mag(vector));

  return vector;
}

int GetSignal(float num) {
  if (num == 0) return 0;
  return num > 0 ? 1 : -1;
}

// Constant definitions
#define PI 3.14159265358979323846

// Simulation variables
#define width  160 
#define height 44 

#define deltaTime 1. / 60

#define numParticles 200
#define particleMass 1

#define smoothingLength 5 
#define collDamping    .6
#define velTolerance    1
#define pressureMult  200
#define targetDensity   0

Vector* gravity;

// Particle vectors
Vector* positions[numParticles];
Vector* velocities[numParticles];
float densities[numParticles];

// Buffer for drawing
char buffer[width * height];

// Characters drawn
int densityASCII[] = { '.', '*', '#', '&', '8', '%', '@' };
int backgroundASCII = ' ';

float initialDst = 1;

void CheckBoundCollisions() {
  for (int i = 0; i < numParticles; i++) {
  /* printf("%f\n", positions[i] -> x); */
    if (fabs(positions[i] -> x) >= (float) width / 4) {
      if (MagSqrd(velocities[i]) <= pow(velTolerance, 2)) Mult(velocities[i], 0);

      float boundSignal = GetSignal(positions[i] -> x);

      positions[i]  -> x = (float) width / 4 * boundSignal;
      velocities[i] -> x *= collDamping - 1;
    }

    if (fabs(positions[i] -> y) >= (float) height / 2) {
      if (MagSqrd(velocities[i]) <= pow(velTolerance, 2)) Mult(velocities[i], 0);

      float boundSignal = GetSignal(positions[i] -> y);

      positions[i]  -> y = (float) height / 2 * boundSignal;
      velocities[i] -> y *= collDamping - 1;
    }
  }
}

float SmoothingKernel(float distance, float radius) {
  float circDist = fabs(radius - distance); 

  float volume = PI * pow(radius, 4) / 2;
  return pow(circDist, 3) / volume;
}

float SmoothingKernelDer(float distance, float radius) {
  float circDist = fabs(radius - distance);

  float volume = PI * pow(radius, 4) / 2;
  return -3 * pow(circDist, 2) / volume;
}

float CalculateDensity(int particleIndex) {
  // TODO: Consider underestimation of density on surface
  float density = 0;
  for (int j = 0; j <= numParticles; j++) {
    if (particleIndex == j) continue;

    Vector* distVector = CopyVector(positions[particleIndex]);
    Mult(distVector, -1);
    Sum(distVector, positions[j]);

    float distance = Mag( distVector );
    density += particleMass * SmoothingKernel(distance, smoothingLength);

    free(distVector);
  }

  return density;
}

Vector* CalculatePressure(int particleIndex) {
  Vector* totalPressure = NewVector(0, 0);

  for (int j = 0; j < numParticles; j++) {
    if (j == particleIndex) continue;

    Vector* pressureVector = CopyVector(positions[particleIndex]);
    Mult(pressureVector, -1);
    Sum(pressureVector, positions[j]);

    float distance = Mag(pressureVector);
    if (!distance) { pressureVector = RandomDir(); distance = 0.001; } 

    float pressureForce = (densities[particleIndex] - targetDensity) * pressureMult;
    
    Mult(pressureVector, particleMass * pressureForce / (distance * densities[j]) * 
                         SmoothingKernelDer(distance, smoothingLength));

    Sum(totalPressure, pressureVector);
  }
  
  return totalPressure;
}

void Fluidify() {
  for (int i = 0; i < numParticles; i++) densities[i] = CalculateDensity(i);

  for (int i = 0; i < numParticles; i++) {
    Sum(velocities[i], gravity); // Gravity acceleration
    
    // Calculating pressure acceleration
    Vector* pressure = CalculatePressure(i);
    Mult(pressure, deltaTime / densities[i]);
    Sum(velocities[i], pressure);

    // Updating positions
    Vector* velCopy = CopyVector(velocities[i]);
    Mult(velCopy, deltaTime); 
    Sum(positions[i], velCopy); 

    free(pressure); free(velCopy);
  }
}

void Start() {
  printf("\x1b[2J");

  gravity = NewVector(0, 50); Mult(gravity, deltaTime);

  // Displaying particles in grid
  int side = sqrt(numParticles);
  for (int i = 0; i < numParticles; i++) {
    positions[i]  = NewVector(i % side, i /  side);
    velocities[i] = NewVector(0, 0);

    Mult(positions[i], initialDst);
    Sum(positions[i], &(Vector){ (float) (initialDst - side) / 2 , (float) (initialDst - side) / 2 });
  }
}

void End() {
  for (int i = 0; i < numParticles; i++) {
    free(positions[i]); free(velocities[i]); 
  }
}

void Update() {
  memset(buffer, backgroundASCII, width * height);

  Fluidify();
  CheckBoundCollisions();

  // Buffer
  for (int i = 0; i < numParticles; i++) {
    // Updating buffer
    int x = (int) positions[i] -> x * 2 + (float) width  / 2;
    int y = (int) positions[i] -> y     + (float) height / 2; 

    int bufferIdx = x + y * width;
    if (bufferIdx < 0 || bufferIdx > width * height) continue;
    if (buffer[bufferIdx] == backgroundASCII) {
      buffer[bufferIdx] = densityASCII[0]; continue;
    }

    for (int k = 0; k < 7; k++) {
      if (buffer[bufferIdx] != densityASCII[k] || k == 7) continue;
      buffer[bufferIdx] = densityASCII[k + 1]; break;
    }
  }

  printf("\x1b[H");
  for (int k = 0; k < width * height; k++) putchar(k % width ? buffer[k] : 10);
  
  usleep(deltaTime * powf(10, 6));
}

int main() {
  Start();
  while (true) Update();
  End();

  return 0;
}

