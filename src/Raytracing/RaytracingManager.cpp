#include "RaytracingManager.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32

#include "Buffers/Tools/CommandBufferManager.h"
#include "Scene/Models/MeshContainer.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>

void RaytracingManager::enableFeatures(VkDevice device, VkPhysicalDevice physicalDevice)
{
	// Se obtienen las características y extensiones necesarias
	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProperties2{};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);
	accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &accelerationStructureFeatures;
	vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
	// Se obtienen los punteros a las funciones necesarias para raytracing
	vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
	vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
	vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
	vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
	vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
	vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
	vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
	vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
	vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
	vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
}

void RaytracingManager::createBottomLevelAccelerationStructures(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool,
																VkQueue graphicsQueue, std::vector<MeshContainer> sceneMeshes)
{
	int indexOffset = 0;
	int vertexOffset = 0;

	for (const auto &mesh : sceneMeshes)
	{
		for (int i = 0; i < mesh.vertexMeshesData.vertexBufferList.size(); i++)
		{
			AccelerationStructure bottomLevelAccelerationStructure;

			// Primero se toma la geometría que se va a almacenar en la estructura de aceleración
			VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
			VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

			vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(device, mesh.vertexMeshesData.vertexBufferList[i]);
			indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(device, mesh.vertexMeshesData.indexBufferList[i]);

			uint32_t numTriangles = static_cast<uint32_t>(mesh.vertexMeshesData.indices[i].size()) / 3;

			VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
			accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // Geometría opaca //
			accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
			accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
			accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
			accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
			accelerationStructureGeometry.geometry.triangles.maxVertex = mesh.vertexMeshesData.vertices[i].size() - 1; // Índice del último vértice de la lista
			accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);							   // Tamaño de los vértices
			accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT16;
			accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
			accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
			accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

			VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
			accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			accelerationStructureBuildGeometryInfo.geometryCount = 1;
			accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

			VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
			accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
			vkGetAccelerationStructureBuildSizesKHR(
				device,
				VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
				&accelerationStructureBuildGeometryInfo,
				&numTriangles,
				&accelerationStructureBuildSizesInfo);

			// Utilizando todas las características definidas, se crea la estructura de aceleración
			createAccelerationStructure(device, physicalDevice, bottomLevelAccelerationStructure, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, accelerationStructureBuildSizesInfo);

			// Se crea un buffer temporal para que se puedan realizar los cálculos internos con la creación de la estructura de aceleración
			ScratchBuffer scratchBuffer = createScratchBuffer(device, physicalDevice, accelerationStructureBuildSizesInfo.buildScratchSize);

			VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
			accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
			accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
			accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
			accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAccelerationStructure.handle;
			accelerationBuildGeometryInfo.geometryCount = 1;
			accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
			accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

			// No es necesario establecer un offset, ya que la posición es el primer dato que se encuentra en los vértices
			VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
			accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
			accelerationStructureBuildRangeInfo.primitiveOffset = 0;
			accelerationStructureBuildRangeInfo.firstVertex = 0;
			accelerationStructureBuildRangeInfo.transformOffset = 0;
			std::vector<VkAccelerationStructureBuildRangeInfoKHR *> accelerationBuildStructureRangeInfos = {&accelerationStructureBuildRangeInfo};

			// Se crea un command buffer independiente para la creación de la estructura de aceleración
			VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
			vkCmdBuildAccelerationStructuresKHR(
				commandBuffer,
				1,
				&accelerationBuildGeometryInfo,
				accelerationBuildStructureRangeInfos.data());

			CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);
			deleteScratchBuffer(device, scratchBuffer);

			// Se añade la estructura de aceleración a la lista
			bottomLevelAccelerationStructures.push_back(bottomLevelAccelerationStructure);

			// Se añade el offset a la lista
			sceneStructure.blasIndexOffsets.push_back(indexOffset);
			// Se actualiza el offset para la siguiente estructura
			indexOffset += numTriangles;

			// Se añaden los índices de los vértices de cada primitiva de la blas actual
			for (int j = 0; j < mesh.vertexMeshesData.indices[i].size(); j += 3)
			{
				glm::vec3 primitiveVertexIndexes = glm::vec3(mesh.vertexMeshesData.indices[i][j],
															 mesh.vertexMeshesData.indices[i][j + 1],
															 mesh.vertexMeshesData.indices[i][j + 2]);

				sceneStructure.scenePrimitivesIndexes.push_back(primitiveVertexIndexes);
			}

			// Se añade el offset de vértices a la lista
			sceneStructure.blasVertexOffsets.push_back(vertexOffset);
			// Se actualiza el offset para la siguiente estructura
			vertexOffset += mesh.vertexMeshesData.vertices[i].size();

			// Se añaden los vértices a la lista
			for (int j = 0; j < mesh.vertexMeshesData.vertices[i].size(); j++)
			{
				Vertex_Buffer vertexData;
				vertexData.pos = mesh.vertexMeshesData.vertices[i][j].pos;
				vertexData.texCoord = mesh.vertexMeshesData.vertices[i][j].texCoord;
				vertexData.normal = mesh.vertexMeshesData.vertices[i][j].normal;
				vertexData.idMaterial = mesh.vertexMeshesData.vertices[i][j].idMaterial;

				sceneStructure.sceneVertex.push_back(vertexData);
			}
		}
	}
}

void RaytracingManager::createTopLevelAccelerationStructure(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	std::vector<VkAccelerationStructureInstanceKHR> instances;

	for (size_t i = 0; i < bottomLevelAccelerationStructures.size(); ++i)
	{
		VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f};

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transformMatrix;
		instance.instanceCustomIndex = static_cast<uint32_t>(i); // Puedes usar esto como índice único para shaders
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = bottomLevelAccelerationStructures[i].deviceAddress;

		instances.push_back(instance);
	}

	// Crear buffer para las instancias
	VkDeviceSize instancesSize = sizeof(VkAccelerationStructureInstanceKHR) * instances.size();
	VkBuffer instancesBuffer;
	VkDeviceMemory instancesBufferMemory;
	void *instancesBufferMemoryMapped = nullptr;

	BufferCreator::createBuffer(device, physicalDevice, instancesSize,
								VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instancesBuffer, instancesBufferMemory);

	vkMapMemory(device, instancesBufferMemory, 0, instancesSize, 0, &instancesBufferMemoryMapped);
	memcpy(instancesBufferMemoryMapped, instances.data(), instancesSize);
	vkUnmapMemory(device, instancesBufferMemory);

	VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
	instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(device, instancesBuffer);

	// Geometría de tipo instancia
	VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
	accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
	accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

	// Info de construcción
	VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
	buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.pGeometries = &accelerationStructureGeometry;

	uint32_t primitiveCount = static_cast<uint32_t>(instances.size());

	VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
	buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	vkGetAccelerationStructureBuildSizesKHR(
		device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildGeometryInfo,
		&primitiveCount,
		&buildSizesInfo);

	createAccelerationStructure(device, physicalDevice, topLevelAccelerationStructure, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, buildSizesInfo);

	ScratchBuffer scratchBuffer = createScratchBuffer(device, physicalDevice, buildSizesInfo.buildScratchSize);

	buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	buildGeometryInfo.dstAccelerationStructure = topLevelAccelerationStructure.handle;
	buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

	VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
	buildRangeInfo.primitiveCount = primitiveCount;
	buildRangeInfo.primitiveOffset = 0;
	buildRangeInfo.firstVertex = 0;
	buildRangeInfo.transformOffset = 0;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR *> buildRangeInfos = {&buildRangeInfo};

	VkCommandBuffer commandBuffer = CommandBufferManager::beginSingleTimeCommands(commandPool, device);
	vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, buildRangeInfos.data());
	CommandBufferManager::endSingleTimeCommands(commandBuffer, graphicsQueue, device, commandPool);

	deleteScratchBuffer(device, scratchBuffer);

	vkDestroyBuffer(device, instancesBuffer, nullptr);
	vkFreeMemory(device, instancesBufferMemory, nullptr);
}

uint64_t RaytracingManager::getBufferDeviceAddress(VkDevice device, VkBuffer buffer)
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
	bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAI.buffer = buffer;
	return vkGetBufferDeviceAddressKHR(device, &bufferDeviceAI);
}

void RaytracingManager::createAccelerationStructure(VkDevice device, VkPhysicalDevice physicalDevice, AccelerationStructure &accelerationStructure, VkAccelerationStructureTypeKHR type,
													VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
{
	// Creación del buffer y reserva de memoria
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &accelerationStructure.buffer);

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(device, accelerationStructure.buffer, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = BufferCreator::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDevice);

	vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &accelerationStructure.memory);
	vkBindBufferMemory(device, accelerationStructure.buffer, accelerationStructure.memory, 0);

	// Objeto con la estructura de aceleración
	VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
	accelerationStructureCreate_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreate_info.buffer = accelerationStructure.buffer;
	accelerationStructureCreate_info.size = buildSizeInfo.accelerationStructureSize;
	accelerationStructureCreate_info.type = type;
	vkCreateAccelerationStructureKHR(device, &accelerationStructureCreate_info, nullptr, &accelerationStructure.handle);

	VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
	accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	accelerationDeviceAddressInfo.accelerationStructure = accelerationStructure.handle;
	accelerationStructure.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &accelerationDeviceAddressInfo);
}

ScratchBuffer RaytracingManager::createScratchBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize deviceSize)
{
	ScratchBuffer scratchBuffer{};

	// Creación del buffer y reserva de memoria
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = deviceSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	vkCreateBuffer(device, &bufferCreateInfo, nullptr, &scratchBuffer.handle);

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(device, scratchBuffer.handle, &memoryRequirements);
	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = BufferCreator::findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, physicalDevice);

	vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &scratchBuffer.memory);
	vkBindBufferMemory(device, scratchBuffer.handle, scratchBuffer.memory, 0);

	VkBufferDeviceAddressInfoKHR bufferDeviceAddresInfo{};
	bufferDeviceAddresInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAddresInfo.buffer = scratchBuffer.handle;
	scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(device, &bufferDeviceAddresInfo);

	return scratchBuffer;
}

void RaytracingManager::deleteScratchBuffer(VkDevice device, ScratchBuffer &scratchBuffer)
{
	if (scratchBuffer.handle != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(device, scratchBuffer.handle, nullptr);
	}
	if (scratchBuffer.memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, scratchBuffer.memory, nullptr);
	}
}

AccelerationStructure RaytracingManager::getTLAS()
{
	return this->topLevelAccelerationStructure;
}

SceneOrganizationStructure RaytracingManager::getSceneStructure()
{
	return this->sceneStructure;
}

void RaytracingManager::cleanup(VkDevice device)
{
	for (const auto &bottomLevelAccelerationStructure : bottomLevelAccelerationStructures)
	{
		vkDestroyBuffer(device, bottomLevelAccelerationStructure.buffer, nullptr);
		vkFreeMemory(device, bottomLevelAccelerationStructure.memory, nullptr);
		vkDestroyAccelerationStructureKHR(device, bottomLevelAccelerationStructure.handle, nullptr);
	}

	vkDestroyBuffer(device, topLevelAccelerationStructure.buffer, nullptr);
	vkFreeMemory(device, topLevelAccelerationStructure.memory, nullptr);
	vkDestroyAccelerationStructureKHR(device, topLevelAccelerationStructure.handle, nullptr);
}