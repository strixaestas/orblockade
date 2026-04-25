#pragma once
#include <raylib.h>
#define GLSL_VERSION 330

class Skybox {
	Model model;
public:
	Skybox(const char* filename);
	~Skybox();
	void draw(Camera* camera);
};
