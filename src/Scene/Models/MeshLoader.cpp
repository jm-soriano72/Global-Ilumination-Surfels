#include "MeshLoader.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <string>
#include <stdexcept>

void MeshLoader::loadModel(const std::string& filePath, std::vector<std::vector<Vertex>>* vertices, std::vector<std::vector<uint16_t>>* indices, uint32_t* materialIndex) {
    // Se crea un importador de la librería assimp
    Assimp::Importer importer;
    // Se carga el modelo desde la ruta dada
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_MakeLeftHanded |
        aiProcess_FlipUVs |            
        aiProcess_FlipWindingOrder |
        aiProcess_PreTransformVertices);
    // En caso de que no se cargue correctamente, se lanza una excepción
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("Error al cargar el modelo: " + std::string(importer.GetErrorString()));
    }
    // Procesar las mallas de la escena, para obtener sus datos
    processMeshes(scene, vertices, indices, materialIndex);
}

void MeshLoader::processMeshes(const aiScene* scene, std::vector<std::vector<Vertex>>* vertices, std::vector<std::vector<uint16_t>>* indices, uint32_t* materialIndex) {
    // Variables auxiliares para detectar si un modelo tiene multi-material
    uint32_t lastMeshMaterialId = 0;
    uint32_t textureIndex = (*materialIndex);
    // Se recorren todas las mallas de la escena
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        // Se inicializa el vector de vértices y de índices
        std::vector<Vertex> listaVertices;
        std::vector<uint16_t> listaIndices;
        (*vertices).push_back(listaVertices);
        (*indices).push_back(listaIndices);
        // Se obtiene la malla actual
        aiMesh* mesh = scene->mMeshes[i];
        // Índices para recorrer los distintos conjuntos de información de la malla
        unsigned int faceIndex = 0; // Caras, para extraer los índices

        // Vértices, posición
        if (mesh->HasPositions()) {
            // Se recorren los vértices de la malla
            for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                // Se crea un nuevo vértice
                Vertex nVertex;
                // Se lee la posición del vértice actual de la malla
                const aiVector3D* vertexMeshPos = &mesh->mVertices[j];
                // Se pasa esa información a un vec3 para poder asignarla al nuevo vértice
                glm::vec3 vertexPosition = {vertexMeshPos->x, vertexMeshPos->y, vertexMeshPos->z};
                nVertex.pos = vertexPosition;
                // Se añade el vértice al vector global
                (*vertices)[i].push_back(nVertex);
            }
        }

        // Normales. Por el momento, se almacenan como color del modelo
        if (mesh->HasNormals()) {
            // De nuevo, se recorren todos los vértices de la malla
            for (unsigned int k = 0; k < mesh->mNumVertices; k++) {
                // Se lee la normal del vértice actual
                const aiVector3D* normal = &mesh->mNormals[k];
                // Se pasa esa información a un vec3 para poder asignarla al nuevo vértice
                glm::vec3 vertexNormal = { normal->x, normal->y, normal->z };
                // Se actualiza la información del vértice
                (*vertices)[i].at(k).normal = vertexNormal;
            }
        }

        // Coordenadas de textura
        if (mesh->mTextureCoords != NULL) {
            // Se recorren todos los vértice de la malla
            for (unsigned int l = 0; l < mesh->mNumVertices; l++) {
                // Se leen las coordenadas de textura del vértice actual
                const aiVector3D* textCoord = &mesh->mTextureCoords[0][l];
                // Se pasa la información a un vec2 para poder asignarla al nuevo vértice
                glm::vec2 vertexTextCoord = { textCoord->x, textCoord->y };
                // Se actualiza la información del vértice
                (*vertices)[i].at(l).texCoord = vertexTextCoord;
            }
        }

        // Índice del material
        if (mesh->mMaterialIndex > lastMeshMaterialId && (*materialIndex) < MULTI_OBJECT_LIMIT_INDEX) {
            (*materialIndex)++; // Significa que el modelo tiene más de un material
            lastMeshMaterialId = mesh->mMaterialIndex;
        }

        // Se asigna el índice del material a cada vértice de la malla
        for(unsigned int m = 0; m < mesh->mNumVertices; m++) {
            (*vertices)[i].at(m).idMaterial = textureIndex + mesh->mMaterialIndex;
        }

        // Índices de las caras
        if (mesh->HasFaces()) {
            // Se recorren todas las caras de la malla
            for (unsigned int t = 0; t < mesh->mNumFaces; t++) {
                // Se obtiene la cara actual
                const aiFace* face = &mesh->mFaces[t];
                // Se copian los datos al vector de índices
                for (unsigned int s = 0; s < face->mNumIndices; s++) {
                    (*indices)[i].push_back(face->mIndices[s]);
                }
            }
        }

    }
    // Se incrementa el índice del material para el siguiente objeto
    (*materialIndex)++;      
}