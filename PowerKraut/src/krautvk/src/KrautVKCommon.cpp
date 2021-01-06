/*
Copyright 2018 Jonathan Crockett

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "KrautVKCommon.h"

namespace KVKBase {

    Com::KrautCommon Com::kraut;

    std::string Tools::rootPath = std::string("");

    void Tools::findAndReplace(std::string &str, const std::string &find, const std::string &replace) {
        if (find.empty())
            return;

        size_t startPos = 0;

        while ((startPos = str.find(find, startPos)) != std::string::npos) {

            str.replace(startPos, find.length(), replace);

            startPos += replace.length();
        }
    }

    std::vector<char> Tools::getBinaryData(std::string const &filename) {

        std::ifstream file(filename, std::ios::binary);
        if (file.fail()) {
            return std::vector<char>();
        }

        std::streampos begin, end;
        begin = file.tellg();
        file.seekg(0, std::ios::end);
        end = file.tellg();

        std::vector<char> result(static_cast<size_t>(end - begin));
        file.seekg(0, std::ios::beg);
        file.read(&result[0], end - begin);
        file.close();

        return result;

    }

    std::vector<char> Tools::getImageData(std::string const &filename, int requestedComponents, int *width, int *height, int *components, int *dataSize) {
        std::vector<char> fileData = getBinaryData(filename);
        if (fileData.empty()) {
            return std::vector<char>();
        }

        int tmpWidth = 0, tmpHeight = 0, tmpComponents = 0;
        unsigned char *imageData = stbi_load_from_memory(reinterpret_cast<unsigned char *>(&fileData[0]),
                                                      static_cast<int>(fileData.size()), &tmpWidth, &tmpHeight,
                                                      &tmpComponents, requestedComponents);
        if ((imageData == nullptr) ||
            (tmpWidth <= 0) ||
            (tmpHeight <= 0) ||
            (tmpComponents <= 0)) {
            return std::vector<char>();
        }

        int size = (tmpWidth) * (tmpHeight) * (requestedComponents <= 0 ? tmpComponents : requestedComponents);
        if (dataSize) {
            *dataSize = size;
        }
        if (width) {
            *width = tmpWidth;
        }
        if (height) {
            *height = tmpHeight;
        }
        if (components) {
            *components = tmpComponents;
        }

        std::vector<char> output(size);
        memcpy(&output[0], imageData, size);

        stbi_image_free(imageData);
        return output;
    }

    std::array<float, 16> Tools::getProjMatrixPerspective(float const aspectRatio, float const fieldOfView, float const nearClip, float const farClip) {
        float f = 1.0f / std::tan(fieldOfView * 0.5f * 0.01745329251994329576923690768489f);

        return {
                f / aspectRatio,
                0.0f,
                0.0f,
                0.0f,

                0.0f,
                -f,
                0.0f,
                0.0f,

                0.0f,
                0.0f,
                farClip / (nearClip - farClip),
                -1.0f,

                0.0f,
                0.0f,
                (nearClip * farClip) / (nearClip - farClip),
                0.0f
        };
    }

    std::array<float, 16> Tools::getProjMatrixOrtho(float const leftPlane, float const rightPlane, float const topPlane, float const bottomPlane, float const nearPlane, float const farPlane) {
        return {
                2.0f / (rightPlane - leftPlane),
                0.0f,
                0.0f,
                0.0f,

                0.0f,
                2.0f / (bottomPlane - topPlane),
                0.0f,
                0.0f,

                0.0f,
                0.0f,
                1.0f / (nearPlane - farPlane),
                0.0f,

                -(rightPlane + leftPlane) / (rightPlane - leftPlane),
                -(bottomPlane + topPlane) / (bottomPlane - topPlane),
                nearPlane / (nearPlane - farPlane),
                1.0f
        };
    }

    GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule> Tools::loadShader(std::string const &filename) {
        const std::vector<char> spv = Tools::getBinaryData(filename);
        if(spv.empty()) {
            return GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule>();
        }

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,    // VkStructureType                sType
                nullptr,                                        // const void                    *pNext
                0,                                              // VkShaderModuleCreateFlags      flags
                spv.size(),                                     // size_t                         codeSize
                reinterpret_cast<const uint32_t*>(&spv[0])      // const uint32_t                *pCode
        };

        VkShaderModule shaderModule;
        if(createShaderModule(Com::kraut.Vulkan.Device.Handle, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule>();
        }

        return GarbageCollector<VkShaderModule, PFN_vkDestroyShaderModule>(shaderModule, destroyShaderModule, Com::kraut.Vulkan.Device.Handle);

    }

    void Com::RenderingResourcesData::DestroyResources() {
        //Destroy Framebuffer
        if (Framebuffer != VK_NULL_HANDLE)
            destroyFramebuffer(Com::kraut.Vulkan.Device.Handle, Framebuffer, nullptr);

        //Destroy Command Buffer
        if (CommandBuffer != VK_NULL_HANDLE)
            freeCommandBuffers(Com::kraut.Vulkan.Device.Handle, Com::kraut.Vulkan.CommandPool, 1, &CommandBuffer);

        //Destroy Semaphores
        if (ImageAvailableSemaphore != VK_NULL_HANDLE)
            destroySemaphore(Com::kraut.Vulkan.Device.Handle, ImageAvailableSemaphore, nullptr);

        if (FinishedRenderingSemaphore != VK_NULL_HANDLE)
            destroySemaphore(Com::kraut.Vulkan.Device.Handle, FinishedRenderingSemaphore, nullptr);

        if (Fence != VK_NULL_HANDLE)
            destroyFence(Com::kraut.Vulkan.Device.Handle, Fence, nullptr);
    }
}

