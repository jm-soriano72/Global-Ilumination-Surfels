#pragma once

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

class CameraController
{
public:
    // Función que gestiona los inputs del ratón, para controlar la cámara
    static void mouseCallback(GLFWwindow *window, double xpos, double ypos);
    // Función que controla el zoom del ratón
    static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    // Función que imprime los datos de la cámara, para saber dónde se sitúa
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};