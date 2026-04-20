#pragma once
#include <raylib.h>

class Skybox {
	Model model;
public:
	Skybox(const char* filename);
	void draw(Camera* camera);
};
