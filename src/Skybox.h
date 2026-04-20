#pragma once
#include <raylib.h>

class Skybox {
	Model model;
public:
	Skybox(const char* filename);
	~Skybox();
	void draw(Camera* camera);
};
