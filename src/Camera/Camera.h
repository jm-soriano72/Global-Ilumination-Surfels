#pragma once

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

class Camera
{
private:
    const glm::vec3 STARTING_TARGET = glm::vec3(-2000.0f, 300.0f, 0.0f);
    const glm::vec3 STARTING_POSITION = glm::vec3(1150.0f, 135.0f, 0.0f);
    glm::vec3 target;   // Objetivo de la cámara
    glm::vec3 position; // Posición de la cámara
    float distance;     // Distancia entre la cámara y el objetivo al que mira
    float yaw, pitch;   // Rotaciones que aplican los inputs del teclado a la posición de la cámara
    float sensitivity;  // Sensibilidad del ratón
    float zoomSpeed;    // Velocidad del zoom

    // Se calcula la rotación inicial en base a la posición y al objetivo predefinidos
    void calculateInitialYawPitch();
    // Actualiza la posición de la cámara basada en la rotación captada
    void updateCameraPosition();

public:
    Camera();
    // Rotación en base al input del ratón
    void processMouseOrbit(float xoffset, float yoffset);
    // Se trasladan la cámara y el objetivo de forma lineal
    void processMousePan(float xoffset, float yoffset);
    // Zoom
    void processScroll(float yoffset);

    glm::vec3 getForwardVector();
    glm::mat4 getViewMatrix();
    glm::vec3 getPosition();
    glm::vec3 getTarget();
};