#include "Camera.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS

Camera::Camera() : target(STARTING_TARGET), position(STARTING_POSITION),
                   sensitivity(0.1f), zoomSpeed(10.0f)
{
    distance = glm::distance(position, target);
    calculateInitialYawPitch();
}

void Camera::calculateInitialYawPitch()
{
    glm::vec3 direction = glm::normalize(target - position);

    pitch = glm::degrees(asin(direction.y));
    yaw = glm::degrees(atan2(direction.z, direction.x));
}

void Camera::updateCameraPosition()
{
    glm::vec3 offset;
    offset.x = distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    offset.y = distance * sin(glm::radians(pitch));
    offset.z = distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    position = target - offset;
}

void Camera::processMouseOrbit(float xoffset, float yoffset)
{
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    // Se evita que la cámara gire completamente
    pitch = glm::clamp(pitch, -30.0f, 5.0f);

    updateCameraPosition();
}

void Camera::processMousePan(float xoffset, float yoffset)
{
    glm::vec3 right = glm::normalize(glm::cross(getForwardVector(), glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, getForwardVector()));

    target += right * xoffset * sensitivity;
    target += up * yoffset * sensitivity;

    updateCameraPosition();
}

void Camera::processScroll(float yoffset)
{
    distance -= yoffset * zoomSpeed;
    updateCameraPosition();
}

glm::vec3 Camera::getForwardVector()
{
    return glm::normalize(target - position);
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::getPosition()
{
    return position;
}

glm::vec3 Camera::getTarget() 
{
    return target;
}