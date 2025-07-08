#pragma once

#include "Camera/Camera.h"
#include "Camera/CameraController.h"

#include <GLFW/glfw3.h>

#include <string>
#include <stdexcept>

class WindowManager
{

private:
    GLFWwindow *window;
    bool framebufferResized; // Indica si el framebuffer ha sido redimensionado
    Camera *camera;          // Referencia a la cámara de la escena

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

public:
    WindowManager();

    void initWindow(uint32_t width, uint32_t height);
    void cleanupWindow();

    GLFWwindow* getWindow() const;
    Camera* getCamera() const;
    bool isFramebufferResized() const;
    void resetFramebufferResized();
};