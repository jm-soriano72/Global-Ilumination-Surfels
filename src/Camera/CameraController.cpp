#include "CameraController.h"

#include "Camera.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

void CameraController::mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    static bool firstMouse = true;
    static float lastX = 800 / 2.0f, lastY = 600 / 2.0f;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Invertido para mantener la dirección correcta
    lastX = xpos;
    lastY = ypos;

    Camera *camera = reinterpret_cast<Camera *>(glfwGetWindowUserPointer(window));

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        camera->processMouseOrbit(xoffset, yoffset);
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) // Ahora usa el botón izquierdo
    {
        camera->processMousePan(xoffset, yoffset);
    }
}

// Función que controla el zoom del ratón
void CameraController::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    Camera *camera = reinterpret_cast<Camera *>(glfwGetWindowUserPointer(window));
    camera->processScroll(yoffset);
}

// Función que imprime los datos de la cámara, para saber dónde se sitúa
void CameraController::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        Camera *camera = reinterpret_cast<Camera *>(glfwGetWindowUserPointer(window));
        std::cout << "Posición de la cámara: ("
                  << camera->getPosition().x << ", "
                  << camera->getPosition().y << ", "
                  << camera->getPosition().z << ")" << std::endl;
        std::cout << "Objetivo de la cámara: ("
                  << camera->getTarget().x << ", "
                  << camera->getTarget().y << ", "
                  << camera->getTarget().z << ")" << std::endl;
    }
}