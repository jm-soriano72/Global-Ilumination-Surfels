#include "WindowManager.h"

#include "Camera/Camera.h"
#include "Camera/CameraController.h"

#include <GLFW/glfw3.h>

#include <string>
#include <stdexcept>

WindowManager::WindowManager() {
    framebufferResized = false;
}

void WindowManager::initWindow(uint32_t width, uint32_t height)
{
    // Primero se inicializa la librería
    glfwInit();
    // GLFW suele tratar con OpenGL, por lo que se debe evitar que se cree un contexto tal
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Tras establecer la configuración, se crea la ventana y se almacena una referencia a ella
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    // Se crea la cámara de la escena
    camera = new Camera();
    // Se pasa la referencia a la cámara
    glfwSetWindowUserPointer(window, camera);
    // Se asignan los callbacks asociados a los inputs del usuario, para poder manejar la cámara
    glfwSetCursorPosCallback(window, CameraController::mouseCallback);
    glfwSetScrollCallback(window, CameraController::scrollCallback);
    glfwSetKeyCallback(window, CameraController::keyCallback);
    // Funcion que se llamará cuando se redimensione la ventana
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void WindowManager::cleanupWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void WindowManager::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto wm = reinterpret_cast<WindowManager *>(glfwGetWindowUserPointer(window));
    wm->framebufferResized = true;
}

GLFWwindow* WindowManager::getWindow() const { return this->window; }
Camera* WindowManager::getCamera() const { return this->camera; }
bool WindowManager::isFramebufferResized() const { return this->framebufferResized; }
void WindowManager::resetFramebufferResized() { this->framebufferResized = false; }