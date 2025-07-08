#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vector>

const uint32_t NUM_TEXTURES_PER_MATERIAL = 3;
// Rutas de los modelos de la escena
const std::vector<char *> meshesPaths = {
    RESOURCES_PATH "models/1 - Hojas.fbx",
    RESOURCES_PATH "models/2 - Tiestos.fbx",
    RESOURCES_PATH "models/3 - Adornos leon.fbx",
    RESOURCES_PATH "models/4 - Paredes.fbx",
    RESOURCES_PATH "models/5 - Arcos y adornos.fbx",
    RESOURCES_PATH "models/6 - Bovedas.fbx",
    RESOURCES_PATH "models/7 - Columnas A.fbx",
    RESOURCES_PATH "models/8 - Suelo.fbx",
    RESOURCES_PATH "models/9 - Columnas C.fbx",
    RESOURCES_PATH "models/10 - Detalles ventanas.fbx",
    RESOURCES_PATH "models/11 - Columnas B.fbx",
    RESOURCES_PATH "models/12 - Lanzas.fbx",
    RESOURCES_PATH "models/13 - Tela verde.fbx",
    RESOURCES_PATH "models/14 - Tela azul.fbx",
    RESOURCES_PATH "models/15 - Tela roja.fbx",
    RESOURCES_PATH "models/16 - Tela azul escudo.fbx",
    RESOURCES_PATH "models/17 - Tela roja escudo.fbx",
    RESOURCES_PATH "models/18 - Tela verde escudo.fbx",
    RESOURCES_PATH "models/19 - Cadenas.fbx",
    RESOURCES_PATH "models/20 - Candelabros.fbx",
};
// Vector con las rutas de las texturas difusas
const std::vector<char *> diffuseTexturesPath = {
    RESOURCES_PATH "textures/1 - Hojas.png",
    RESOURCES_PATH "textures/2 - Plantas.png",
    RESOURCES_PATH "textures/2 - Tiesto.png",
    RESOURCES_PATH "textures/3 - Adornos leon.png",
    RESOURCES_PATH "textures/3 - Leon.png",
    RESOURCES_PATH "textures/4 - Paredes.png",
    RESOURCES_PATH "textures/5 - Arcos.png",
    RESOURCES_PATH "textures/6 - Bovedas.png",
    RESOURCES_PATH "textures/7 - Columnas A.png",
    RESOURCES_PATH "textures/8 - Suelo.png",
    RESOURCES_PATH "textures/9 - Columnas C.png",
    RESOURCES_PATH "textures/10 - Detalles ventanas.png",
    RESOURCES_PATH "textures/11 - Columnas B.png",
    RESOURCES_PATH "textures/12 - Lanzas.png",
    RESOURCES_PATH "textures/13 - Tela verde.png",
    RESOURCES_PATH "textures/14 - Tela azul.png",
    RESOURCES_PATH "textures/15 - Tela roja.png",
    RESOURCES_PATH "textures/16 - Tela azul escudo.png",
    RESOURCES_PATH "textures/17 - Tela roja escudo.png",
    RESOURCES_PATH "textures/18 - Tela verde escudo.png",
    RESOURCES_PATH "textures/19 - Cadenas.png",
    RESOURCES_PATH "textures/20 - Candelabros.png",
};
// Vector con las rutas de los mapas de transparencia
const std::vector<char *> alphaTexturesPath = {
    RESOURCES_PATH "textures/1 - Hojas Transparencia.png",
    RESOURCES_PATH "textures/2 - Plantas Transparencia.png",
    RESOURCES_PATH "textures/2 - Tiesto Transparencia.png",
    RESOURCES_PATH "textures/3 - Adornos leon Transparencia.png",
    RESOURCES_PATH "textures/3 - Leon Transparencia.png",
    RESOURCES_PATH "textures/4 - Paredes Transparencia.png",
    RESOURCES_PATH "textures/5 - Arcos Transparencia.png",
    RESOURCES_PATH "textures/6 - Bovedas Transparencia.png",
    RESOURCES_PATH "textures/7 - Columnas A Transparencia.png",
    RESOURCES_PATH "textures/8 - Suelo Transparencia.png",
    RESOURCES_PATH "textures/9 - Columnas C Transparencia.png",
    RESOURCES_PATH "textures/10 - Detalles ventanas Transparencia.png",
    RESOURCES_PATH "textures/11 - Columnas B Transparencia.png",
    RESOURCES_PATH "textures/12 - Lanzas Transparencia.png",
    RESOURCES_PATH "textures/13 - Tela verde Transparencia.png",
    RESOURCES_PATH "textures/14 - Tela azul Transparencia.png",
    RESOURCES_PATH "textures/15 - Tela roja Transparencia.png",
    RESOURCES_PATH "textures/16 - Tela azul escudo Transparencia.png",
    RESOURCES_PATH "textures/17 - Tela roja escudo Transparencia.png",
    RESOURCES_PATH "textures/18 - Tela verde escudo Transparencia.png",
    RESOURCES_PATH "textures/19 - Cadenas Transparencia.png",
    RESOURCES_PATH "textures/20 - Candelabros Transparencia.png",
};
// Vector con las rutas de los mapas especulares
const std::vector<char *> specularTexturesPath = {
    RESOURCES_PATH "textures/1 - Hojas Especular.png",
    RESOURCES_PATH "textures/2 - Plantas Especular.png",
    RESOURCES_PATH "textures/2 - Tiesto Especular.png",
    RESOURCES_PATH "textures/3 - Adornos leon Especular.png",
    RESOURCES_PATH "textures/3 - Leon Especular.png",
    RESOURCES_PATH "textures/4 - Paredes Especular.png",
    RESOURCES_PATH "textures/5 - Arcos Especular.png",
    RESOURCES_PATH "textures/6 - Bovedas Especular.png",
    RESOURCES_PATH "textures/7 - Columnas A Especular.png",
    RESOURCES_PATH "textures/8 - Suelo Especular.png",
    RESOURCES_PATH "textures/9 - Columnas C Especular.png",
    RESOURCES_PATH "textures/10 - Detalles ventanas Especular.png",
    RESOURCES_PATH "textures/11 - Columnas B Especular.png",
    RESOURCES_PATH "textures/12 - Lanzas Especular.png",
    RESOURCES_PATH "textures/13 - Tela verde Especular.png",
    RESOURCES_PATH "textures/14 - Tela azul Especular.png",
    RESOURCES_PATH "textures/15 - Tela roja Especular.png",
    RESOURCES_PATH "textures/16 - Tela azul escudo Especular.png",
    RESOURCES_PATH "textures/17 - Tela roja escudo Especular.png",
    RESOURCES_PATH "textures/18 - Tela verde escudo Especular.png",
    RESOURCES_PATH "textures/19 - Cadenas Especular.png",
    RESOURCES_PATH "textures/20 - Candelabros Especular.png"
};

const std::vector<bool> translucentMaterials = {
    true,
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    true,
    true,
    true,
    true,
    true,
    true,
    false,
    false
};

// Datos para la iluminación
// Posiciones
const std::vector<glm::vec3> lightsPositions = {
    glm::vec3(0.0f, 100.0f, 150.0f),
    glm::vec3(1000.0f, 600.0f, 0.0f),
    glm::vec3(-500.0f, 150.0f, -100.0f),
    glm::vec3(800.0f, 100.0f, 200.0f)};
// Intensidades
const std::vector<float> lightsIntensities = {1000.0f, 5000.0f, 5000.0f, 3000.0f};
// Colores
const std::vector<glm::vec3> lightsColors = {
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.6f, 0.0f)};