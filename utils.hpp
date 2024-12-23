#ifndef _JC_UTILS_HPP
#define _JC_UTILS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h>
#include "include/glm/vec3.hpp"
#include "include/glm/vec2.hpp"
#include <string.h>
#include "platform_common.hpp"

struct BMP { 
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width;
	unsigned int height;
	unsigned int imageSize;
	unsigned char* data;
};

struct BMP loadBMP(const char* filename);
char* readfile(const char* filename) ;
bool loadOBJ(const char* fileName, std::vector<glm::vec3> &vertices, std::vector<glm::vec2> &uvs, std::vector<glm::vec3> &normals);

#endif