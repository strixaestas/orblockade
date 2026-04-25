#include "Skybox.h"
#include <raylib.h>
#include <rlgl.h>

Skybox::Skybox(const char* filename) {
	Mesh skyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
	model = LoadModelFromMesh(skyboxMesh);

    model.materials[0].shader = LoadShader(TextFormat("resources/shaders/glsl%i/skybox.vs", GLSL_VERSION),
                                            TextFormat("resources/shaders/glsl%i/skybox.fs", GLSL_VERSION));

    SetShaderValue(model.materials[0].shader, GetShaderLocation(model.materials[0].shader, "environmentMap"), (int[1]){ MATERIAL_MAP_CUBEMAP }, SHADER_UNIFORM_INT);
    SetShaderValue(model.materials[0].shader, GetShaderLocation(model.materials[0].shader, "doGamma"), (int[1]){0}, SHADER_UNIFORM_INT);
    SetShaderValue(model.materials[0].shader, GetShaderLocation(model.materials[0].shader, "vflipped"), (int[1]){0}, SHADER_UNIFORM_INT);

	Image image = LoadImage(filename);
	model.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(image, CUBEMAP_LAYOUT_AUTO_DETECT);
	UnloadImage(image);
}

Skybox::~Skybox() {
	UnloadModel(model);
}

void Skybox::draw(Camera* camera) {
	rlDisableBackfaceCulling();
	rlDisableDepthMask();
	DrawModel(model, camera->position, 1.0f, WHITE);
	rlEnableBackfaceCulling();
	rlEnableDepthMask();
}
