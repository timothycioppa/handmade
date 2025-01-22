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

#pragma pack(push, 1)
struct bitmap_header
{	
	uint16_t FileType;
	uint32_t FileSize;
	uint16_t Reserved1;
	uint16_t Reserved2;
	uint32_t BitmapOffset;
	uint32_t Size;
	uint32_t Width;
	uint32_t Height;
	uint16_t Planes;
	uint16_t BitsPerPlane;
};
#pragma pack(pop)

struct BMP 
{ 
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width;
	unsigned int height;
	unsigned int imageSize;
	unsigned char* data;
};

// uses scratch memory, primarily used just to load immediately onto the GPU
struct BMP loadBMP_scratch(const char* filename) ;

struct BMP loadBMP(const char* filename);

void freeBMP(BMP & bmp);
char* readfile(const char* filename) ;
char* readfile_scratch(const char* filename);

bool loadOBJ(const char* fileName, std::vector<glm::vec3> &vertices, std::vector<glm::vec2> &uvs, std::vector<glm::vec3> &normals);
bitmap_header load_bitmap(const char* filename) ;

#endif