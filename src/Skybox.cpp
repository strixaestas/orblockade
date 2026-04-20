#include "Skybox.h"
#include <raylib.h>
#include <rlgl.h>

Skybox::Skybox(const char* filename) {
	Mesh skyboxMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
	model = LoadModelFromMesh(skyboxMesh);
	Image image = LoadImage(filename);
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(image);
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
