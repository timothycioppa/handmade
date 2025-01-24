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

struct hash_node 
{
	uint16_t x, y, z;
	int data;	
	hash_node *nextInHash;
};

struct hash_entry 
{ 
	unsigned short count;
	hash_node *node;
};

hash_entry hashEntries[4096];
hash_node nodeTable[256];
unsigned int nodeTableHead = 0;

hash_node *create_new_hash_node(uint16_t x, uint16_t y, uint16_t z, int data) 
{
	hash_node *node = nodeTable + nodeTableHead;
	node->x = x;
	node->y = y;
	node->z = z;
	node->data = data;
	nodeTableHead++;
	return node;
}

bool contains(uint16_t x, uint16_t y, uint16_t z) 
{
	uint32_t hashValue = 7 * x + 13 * y + 19 * z;
	uint32_t hashSlot = hashValue & 4095;
	hash_entry & entry = hashEntries[hashSlot];
	bool result = false;

	if (entry.count > 0) 
	{
		hash_node * node = entry.node;
		while (node) 
		{
			if (node->x == x &&  node->y == y && node->z == z) 
			{
				result = true;
				break;
			}
			node = node->nextInHash;
		}
	}

	return result;
}

bool try_get(uint16_t x, uint16_t y, uint16_t z, int *result) 
{ 
	uint32_t hashValue = 7 * x + 13 * y + 19 * z;
	uint32_t hashSlot = hashValue & 4095;
	hash_entry & entry = hashEntries[hashSlot];
	bool found = false;

	if (entry.count > 0) 
	{
		hash_node * node = entry.node;
		do 
		{
			if (node->x == x && node->y == y && node->z == z) 
			{
				found = true;
				*result = node->data;
				break;
			}
			node = node->nextInHash;
		} while (node);
	}
	return found;
}


void add_hash_data(uint16_t x, uint16_t y, uint16_t z, int data) 
{
	uint32_t hashValue = 7 * x + 13 * y + 19 * z;
	uint32_t hashSlot = hashValue & 4095;
	hash_entry & entry = hashEntries[hashSlot];
	bool found = false;

	if (entry.count > 0) 
	{
		hash_node * node = entry.node;
			
		// infinite loop seems sketchy but we break out in all cases
		// 1. break out if we find a matching node
		// 2. break out as soon as we reach the last node in the list
		for (;;) 
		{
			// override the result which already exists and break out of loop
			if (node->x == x && node->y == y && node->z == z) 
			{
				found = true;
				node->data = data;
				break;
			}

			// end of list and still not found, break out of loop
			if (node->nextInHash == 0) 
			{
				break;	
			}

			// proceed to next entry in the list
			node = node->nextInHash;	
		}

		if (!found) 
		{
			node->nextInHash = create_new_hash_node(x,y,z,data);
			entry.count++;		
		}
	}
	else
	{
		entry.node = create_new_hash_node(x,y,z,data);
		entry.count++;
	}
} 

#endif