#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <webgpu/webgpu.h>
#include <sdl3webgpu.h>

#define WINDOW_TITLE "ORB Engine"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600


// Type for the static app state structure
typedef struct {
    SDL_Window *window;
    WGPUInstance instance;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;
    WGPUTextureFormat textureFormat;
} ORBE_State;

// Type for the adapter callback userdata
typedef struct AdapterUserData {
    WGPUAdapter adapter;
    WGPUBool completed;
} AdapterUserData;


// Handle the callback for requesting a WebGPU adapter
void requestAdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) {
    AdapterUserData* adapterUserData = (AdapterUserData*)userdata1;
    adapterUserData->adapter = adapter; // Store the adapter in userdata1
    adapterUserData->completed = WGPU_TRUE; // Mark the request as completed
}


//
// SDL Application Callbacks
//

int SDL_AppInit(void **appstate, int argc, char *argv[]) {
    // Create a static app state structure and put the pointer to it in *appstate
    static ORBE_State state = {0};
    *appstate = (void *)&state;

    // Create the SDL window
    state.window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (state.window == NULL) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_Log("Window created successfully: %p", (void *)state.window);

    // Initialize WebGPU instance and create a surface
    state.instance = wgpuCreateInstance(NULL);
    state.surface = SDL_GetWGPUSurface(state.instance, state.window);
    if (state.surface == NULL) {
        SDL_Log("Couldn't get WebGPU surface.");
        SDL_DestroyWindow(state.window);
        return SDL_APP_FAILURE;
    }
    SDL_Log("Window surface for WebGPU identified: %p", (void *)state.surface);

    //
    // Obtain the WebGPU adapter and device
    //
    // Initialize adapter data
    AdapterUserData adapterData = {
        .adapter = NULL,
        .completed = WGPU_FALSE
    };
    // Request the adapter with high performance preference and obtained WGPU surface
    WGPURequestAdapterOptions options = {
        .powerPreference = WGPUPowerPreference_HighPerformance,
        .compatibleSurface = state.surface
    };
    // Set up the callback info
    WGPURequestAdapterCallbackInfo callbackInfo = {
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = requestAdapterCallback,
        .userdata1 = &adapterData
    };
    // Request the adapter
    wgpuInstanceRequestAdapter(state.instance, &options, callbackInfo);
    while (adapterData.completed != WGPU_TRUE) {
        wgpuInstanceProcessEvents(state.instance);
    }
    // Check if the adapter was obtained successfully
    if (adapterData.adapter == NULL) {
        SDL_Log("Failed to obtain WebGPU adapter.");
        return SDL_APP_FAILURE;
    }
    SDL_Log("WebGPU adapter obtained.");

    // Create the GPU device
    state.device = wgpuAdapterCreateDevice(adapterData.adapter, NULL);
    // Check if the device was created successfully
    if (state.device == NULL) {
        SDL_Log("Failed to create WebGPU device.");
        return SDL_APP_FAILURE;
    }
    // Log the device information
    WGPUAdapterInfo adapterInfo = {0};
    wgpuDeviceGetAdapterInfo(state.device, &adapterInfo);
    SDL_Log("WebGPU device identified: %.*s, %.*s",
        (int)adapterInfo.device.length, adapterInfo.device.data,
        (int)adapterInfo.description.length, adapterInfo.description.data
    );
    // Store the queue in the state
    state.queue = wgpuDeviceGetQueue(state.device);

    // Configure the surface
    WGPUSurfaceCapabilities capabilities = {0};
    WGPUStatus surfaceGetCapabilitiesStatus = wgpuSurfaceGetCapabilities(state.surface, adapterData.adapter, &capabilities);
    if (surfaceGetCapabilitiesStatus != WGPUStatus_Success) {
        SDL_Log("Failed to get surface capabilities: %d", surfaceGetCapabilitiesStatus);
        return SDL_APP_FAILURE;
    }
    if (capabilities.formatCount == 0) {
        SDL_Log("No supported formats found for the surface.");
        return SDL_APP_FAILURE;
    }
    state.textureFormat = capabilities.formats[0];
    WGPUSurfaceConfiguration surfaceConfiguration = {
        .nextInChain = NULL,
        .device = state.device,
        .format = state.textureFormat,
        .usage = WGPUTextureUsage_RenderAttachment,
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
        .viewFormatCount = 0, // No specific view formats
        .viewFormats = NULL,
        .alphaMode = WGPUCompositeAlphaMode_Auto,
        .presentMode = WGPUPresentMode_Fifo
    };
    SDL_Log("Configuring surface with format: %d, width: %u, height: %u", 
        state.textureFormat, surfaceConfiguration.width, surfaceConfiguration.height
    );
    wgpuSurfaceConfigure(state.surface, &surfaceConfiguration);
    SDL_Log("Surface configured successfully.");

    // Release structure which are not going to be needed anymore
    wgpuSurfaceCapabilitiesFreeMembers(capabilities);
    wgpuAdapterRelease(adapterData.adapter);

    // Successfully initialized the application
    SDL_Log("Application initialized successfully");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ORBE_State *state = (ORBE_State *)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    ORBE_State *state = (ORBE_State *)appstate;

    // Start with processing the WebGPU events
    wgpuInstanceProcessEvents(state->instance);
    
    //
    // Prepare the texture view and color attachment for rendering
    //

    // Get the current texture to draw on from the surface
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(state->surface, &surfaceTexture);
    if (surfaceTexture.texture == NULL) {
        SDL_Log("Failed to get current texture from surface.");
        return SDL_APP_FAILURE;
    }

    // Create a texture view for the surface texture and a color attachment for the render pass
    WGPUTextureViewDescriptor textureViewDescriptor = {
        .nextInChain = NULL,
        .label = "Surface Texture View",
        .format = wgpuTextureGetFormat(surfaceTexture.texture),
        .dimension = WGPUTextureViewDimension_2D,
        .aspect = WGPUTextureAspect_All,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1
    };
    WGPURenderPassColorAttachment colorAttachment = {
        .view = wgpuTextureCreateView(surfaceTexture.texture, &textureViewDescriptor),
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {0.5f, 0.0f, 0.5f, 1.0f}, // Purple clear color
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED
    };
    if (colorAttachment.view == NULL) {
        SDL_Log("Failed to create texture view for render pass.");
        return SDL_APP_FAILURE;
    }

    //
    // Create a command buffer
    //

    // Initialize the command encoder object
    WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(state->device, NULL);
    if (commandEncoder == NULL) {
        SDL_Log("Failed to create command encoder.");
        return SDL_APP_FAILURE;
    }

    // Create the render pass encoder
    WGPURenderPassDescriptor renderPassDescriptior = {
        .colorAttachmentCount = 1,
        .colorAttachments = &colorAttachment
    };
    WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPassDescriptior);
    if (renderPassEncoder == NULL) {
        SDL_Log("Failed to begin render pass.");
        wgpuCommandEncoderRelease(commandEncoder);
        return SDL_APP_FAILURE;
    }
    wgpuRenderPassEncoderEnd(renderPassEncoder);

    // Create a command buffer with all the commands and release the command encoder
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, NULL);
    wgpuCommandEncoderRelease(commandEncoder);

    // Draw on screen - execute the command buffer and present the surface
    wgpuQueueSubmit(state->queue, 1, &commandBuffer);
    wgpuSurfacePresent(state->surface);

    // Release structures that are no longer needed
    wgpuCommandBufferRelease(commandBuffer);
    wgpuTextureViewRelease(colorAttachment.view);

    // Simulate a frame delay (60 FPS)
    SDL_Delay(16);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ORBE_State *state = (ORBE_State *)appstate;

    wgpuQueueRelease(state->queue);
    state->queue = NULL;
    wgpuDeviceRelease(state->device);
    state->device = NULL;
    wgpuInstanceRelease(state->instance);
    state->instance = NULL;
    SDL_DestroyWindow(state->window);
    state->window = NULL;
    wgpuSurfaceRelease(state->surface);
    state->surface = NULL;

    SDL_Log("Application quit with result: %d", result);
}
