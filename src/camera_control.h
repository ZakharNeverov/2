#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#include <GL/glut.h>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>

extern float cameraAngleX;
extern float cameraAngleY;
extern float cameraDistance;

void handleMouse(int button, int state, int x, int y);
void handleMouseMotion(int x, int y);
void handleMouseWheel(int wheel, int direction, int x, int y);
void applyCameraView();

glm::vec3 getCameraPosition();
glm::mat4 getCameraViewMatrix();

#endif
