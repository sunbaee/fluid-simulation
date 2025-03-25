#include <stdbool.h>

#define numParticles 100
#define deltaTime 1 / 60

struct Vector { int x; int y; } typedef Vector;

// Particle vectors
Vector* positions[numParticles];
Vector* velocities[numParticles];
Vector* accelerations[numParticles];

void UpdateParticle(int i) {
  positions[i] -> x += velocities[i] -> x * deltaTime;  
  positions[i] -> y += velocities[i] -> y * deltaTime;  

  velocities[i] -> x += accelerations[i] -> x * deltaTime;  
  velocities[i] -> y += accelerations[i] -> y * deltaTime;  
}

void Update() {
  for (int i = 0; i < numParticles; i++) {
    UpdateParticle(i);
  }  
}

int main() {
  while (true) Update();

  return 0;
}

