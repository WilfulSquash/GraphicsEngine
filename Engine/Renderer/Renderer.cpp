#include "pch.h"
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Pipeline.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include "Math/Transform.h"
#include "Math/MathUtils.h"
#include "Math/Rect.h"

using namespace std;

namespace nu
{
    bool Renderer::Initialize(const char* name, int width, int height)
    {
        m_width = width;
        m_height = height;

        SDL_Init(SDL_INIT_VIDEO);

        if (!TTF_Init()) 
        {
            std::cerr << "TTF_Init Error: " << SDL_GetError() << std::endl;
            return false;
        }

        m_window = SDL_CreateWindow(name, width, height, 0);
        if (m_window == nullptr) 
        {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        SDL_SetHint(SDL_HINT_GPU_DRIVER, "vulkan");
        SDL_GPUShaderFormat formats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL;
        m_gpuDevice = SDL_CreateGPUDevice(formats, true, nullptr);
        if (!m_gpuDevice)
        {
            std::cerr << "Failed to create GPU Device: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            return false;
        }

        // claim the window for our modern GPU context
        SDL_ClaimWindowForGPUDevice(m_gpuDevice, m_window);

        // print out the driver being utilized (e.g., "vulkan" or "d3d12")
        std::cout << "GPU Driver Initialized: " << SDL_GetGPUDeviceDriver(m_gpuDevice) << std::endl;

        CreateDepthBuffer();

        return true;
    }

    void Renderer::Shutdown()
    {
        TTF_Quit();
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool Renderer::BeginFrame()
    {
        m_gpuCommandBuffer = SDL_AcquireGPUCommandBuffer(m_gpuDevice);
        if (!m_gpuCommandBuffer)
        {
            std::cerr << "Could not acquire command buffer: " << SDL_GetError() << std::endl;
            return false;
        }


        SDL_GPUTexture* swapchainTexture = nullptr;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(m_gpuCommandBuffer, m_window, &swapchainTexture, nullptr, nullptr))
        {
            std::cerr << "Could not acquire swapchain texture: " << SDL_GetError() << std::endl;
            return false;
        }

        if (swapchainTexture != nullptr)
        {
            // configure the color target attachments (This handles clearing the screen)
            SDL_GPUColorTargetInfo color_target_info{};
            color_target_info.texture = swapchainTexture;
            color_target_info.clear_color = SDL_FColor{ 0.0f, 0.5f, 1.0f, 1.0f };
            color_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
            color_target_info.store_op = SDL_GPU_STOREOP_STORE;

            // configure the depth target (This handles the depth sorting)
            SDL_GPUDepthStencilTargetInfo depth_target_info = {};
            depth_target_info.texture = m_depthTexture;
            depth_target_info.clear_depth = 1.0f;
            depth_target_info.load_op = SDL_GPU_LOADOP_CLEAR;
            depth_target_info.store_op = SDL_GPU_STOREOP_DONT_CARE;
            depth_target_info.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
            depth_target_info.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

            m_gpuRenderPass = SDL_BeginGPURenderPass(m_gpuCommandBuffer, &color_target_info, 1, &depth_target_info);
        }

        return true;
    }

    bool Renderer::EndFrame() const
    {
            SDL_EndGPURenderPass(m_gpuRenderPass);
        if (!SDL_SubmitGPUCommandBuffer(m_gpuCommandBuffer))
        {
            std::cerr << "Could not submit command buffer: " << SDL_GetError() << std::endl;
            return false;
        }

        return true;
    }

    void Renderer::SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const
    {
        SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
    }

    void Renderer::SetColor(float r, float g, float b, float a) const
    {
        SDL_SetRenderDrawColorFloat(m_renderer, r, g, b, a);
    }

    void Renderer::Clear() const
    {
        SDL_RenderClear(m_renderer);
    }

    void Renderer::Present() const
    {
        SDL_RenderPresent(m_renderer);
    }

    void Renderer::DrawPoint(float x, float y) const
    {
        SDL_RenderPoint(m_renderer, x, y);
    }

    void Renderer::DrawLine(float x1, float y1, float x2, float y2) const
    {
        SDL_RenderLine(m_renderer, x1, y1, x2, y2);
    }

    void Renderer::DrawFillRect(float x, float y, float w, float h) const
    {
        SDL_FRect rect{ x, y, w, h };
        SDL_RenderFillRect(m_renderer, &rect);
    }

    void Renderer::DrawRect(float x, float y, float w, float h) const
    {
        SDL_FRect rect{ x, y, w, h };
        SDL_RenderRect(m_renderer, &rect);
    }

    void Renderer::SetPipeline(const Pipeline& pipeline)
    {
        // bind the pipeline to the current render pass with SDL_BindGPUGraphicsPipeline(renderPass, gpuPipeline)
        // the pipeline sets the shaders and render settings used by the draw calls that follow
        // use pipeline.GetGPUPipeline() to get the sdl pipeline
        SDL_BindGPUGraphicsPipeline(m_gpuRenderPass, pipeline.m_gpuPipeline);
    }

    void Renderer::SetVertexBuffer(const VertexBuffer& vertexBuffer)
    {
        // describe which buffer to bind and where to start reading from
        SDL_GPUBufferBinding binding{
            .buffer = vertexBuffer.m_gpuBuffer, // todo: the vertex buffer's gpu buffer
            .offset = 0   // start reading at the beginning of the buffer
        };

        // bind the vertex buffer to the current render pass with SDL_BindGPUVertexBuffers()
        // the draw calls that follow read their vertices from this buffer
        // parameters:
        //   render pass   - the current render pass (m_renderPass)
        //   first slot    - 0, matches the buffer slot set in Pipeline::AddVertexBuffer()
        //   bindings      - pointer to the binding above
        //   binding count - 1, we are binding one buffer
        SDL_BindGPUVertexBuffers(m_gpuRenderPass, 0, &binding, 1);
    }

    void Renderer::SetIndexBuffer(const IndexBuffer& indexBuffer)
    {
        // describe which buffer to bind and where to start reading from
        SDL_GPUBufferBinding binding{
            .buffer = indexBuffer.m_gpuBuffer, // todo: the index buffer's gpu buffer
            .offset = 0   // start reading at the beginning of the buffer
        };

        // bind the index buffer to the current render pass with SDL_BindGPUIndexBuffer()
        // the draw calls that follow read their indices from this buffer
        // parameters:
        //   render pass   - the current render pass (m_renderPass)
        //   bindings      - pointer to the binding above
        //   binding count - 1, we are binding one buffer
        SDL_BindGPUIndexBuffer(m_gpuRenderPass, &binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
    }

    void Renderer::Draw(uint32_t vertexCount)
    {
        // draw using the currently bound pipeline and vertex buffer with SDL_DrawGPUPrimitives()
        // parameters:
        //   render pass    - the current render pass (m_renderPass)
        //   vertex count   - the number of vertices to draw (vertexCount)
        //   instance count - 1, draw one copy (more than 1 is used for instancing)
        //   first vertex   - 0, start at the first vertex in the buffer
        //   first instance - 0, start at the first instance
        SDL_DrawGPUPrimitives(m_gpuRenderPass, vertexCount, 1, 0, 0);
    }
    void Renderer::DrawIndex(const IndexBuffer& indexBuffer)
    {
		SDL_DrawGPUIndexedPrimitives(m_gpuRenderPass, indexBuffer.GetIndexCount(), 1, 0, 0, 0);
    }
    void Renderer::SetVertexUniform(uint32_t slot, const void* data, uint32_t size)
    {
		SDL_PushGPUVertexUniformData(m_gpuCommandBuffer, slot, data, size);
    }
    void Renderer::SetFragmentUniform(uint32_t slot, const void* data, uint32_t size)
    {
        SDL_PushGPUFragmentUniformData(m_gpuCommandBuffer, slot, data, size);
    }

    void Renderer::SetFragmentTexture(uint32_t slot, const Texture& texture)
    {
        SDL_GPUTextureSamplerBinding binding
        {
            .texture = texture.m_gpuTexture,
            .sampler = texture.m_gpuSampler
        };

        SDL_BindGPUFragmentSamplers(m_gpuRenderPass, slot, &binding, 1);
    }
    void Renderer::CreateDepthBuffer()
    {
        SDL_GPUTextureCreateInfo depthInfo{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = (uint32_t)m_width,
        .height = (uint32_t)m_height,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .sample_count = SDL_GPU_SAMPLECOUNT_1
        };

        m_depthTexture = SDL_CreateGPUTexture(m_gpuDevice, &depthInfo);
    }
}