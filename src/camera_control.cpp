#include "camera_control.h"
#include <cmath>

float cameraAngleX = 45.0f;  // Camera rotation angle around X-axis
float cameraAngleY = 45.0f;  // Camera rotation angle around Y-axis
float cameraDistance = 5.0f; // Distance to the scene

// Mouse states
int lastX = -1, lastY = -1;
bool isMousePressed = false;

void handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        isMousePressed = true;
        lastX = x;
        lastY = y;
    } else {
        isMousePressed = false;
    }
}

void handleMouseMotion(int x, int y) {
    if (isMousePressed) {
        // Adjust camera rotation angles based on mouse movement
        cameraAngleX += (x - lastX) * 0.1f;  // Angular change along X-axis
        cameraAngleY += (y - lastY) * 0.1f;  // Angular change along Y-axis
        lastX = x;
        lastY = y;

        glutPostRedisplay();  // Refresh the scene after changing angles
    }
}

void handleMouseWheel(int wheel, int direction, int x, int y) {
    // Zoom in/out with mouse wheel
    if (direction > 0) {
        cameraDistance -= 0.1f; // Zoom in
    } else {
        cameraDistance += 0.1f; // Zoom out
    }
    if (cameraDistance < 1.0f) cameraDistance = 1.0f; // Minimum distance limit

    glutPostRedisplay();  // Refresh the scene after zooming
}

glm::vec3 getCameraPosition()
{
    // Convert angles from degrees to radians
    float angleXRad = glm::radians(cameraAngleX);
    float angleYRad = glm::radians(cameraAngleY);

    // Calculate camera position
    glm::vec3 cameraPos;
    cameraPos.x = cameraDistance * sin(angleXRad) * cos(angleYRad);
    cameraPos.y = cameraDistance * sin(angleYRad);
    cameraPos.z = cameraDistance * cos(angleXRad) * cos(angleYRad);

    return cameraPos;
}

// Implementation of getCameraViewMatrix()
glm::mat4 getCameraViewMatrix()
{
    glm::vec3 cameraPos = getCameraPosition();
    glm::vec3 target(0.0f, 0.0f, 0.0f); // Looking at the origin
    glm::vec3 up(0.0f, 1.0f, 0.0f);     // Up vector

    // Compute the view matrix
    glm::mat4 view = glm::lookAt(cameraPos, target, up);

    return view;
}

void applyCameraView() {
    // Apply camera view in OpenGL
    glLoadIdentity();
    
    // Camera is moved based on angles
    gluLookAt(cameraDistance * sin(cameraAngleX * M_PI / 180.0f) * cos(cameraAngleY * M_PI / 180.0f),
              cameraDistance * sin(cameraAngleY * M_PI / 180.0f),
              cameraDistance * cos(cameraAngleX * M_PI / 180.0f) * cos(cameraAngleY * M_PI / 180.0f),
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);
}
