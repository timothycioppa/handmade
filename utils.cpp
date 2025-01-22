
#include "utils.hpp"
#include "scratch.hpp"

bitmap_header load_bitmap(const char* filename) 
{
	char* contents = readfile(filename);

	if (contents != NULL) 
	{ 
		bitmap_header* header = (bitmap_header*) contents;
		uint32_t* pixels = (uint32_t*) ((uint8_t*) header + (header->BitmapOffset));
		free(contents);
	}	
	
	bitmap_header result;
	return result;
}


struct BMP loadBMP_scratch(const char* filename) 
{ 
	struct BMP result;	
	result.dataPos = 0;
	result.width = 0;
	result.height = 0;
	result.imageSize = 0;
	result.data = NULL;

	FILE* fp = fopen(filename, "rb");
	
	if (fp == NULL) 
	{ 
		printf("unable to open file [%s]\n", filename);
		return result;
	}


	if (fread(result.header, 1, 54, fp) != 54) 
	{ 
		printf("invalid bmp format [%s]\n", filename);
		fclose(fp);
		return result;
	}

	if (result.header[0] != 'B' || result.header[1] != 'M') 
	{ 
		printf("invalid bmp format [%s]\n", filename);
		fclose(fp);
		return result;
	}

	// Make sure this is a 24bpp file
	if ( *(int*)&(result.header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    fclose(fp); return result;}
	if ( *(int*)&(result.header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    fclose(fp); return result;}

	result.dataPos    = *(int*)&(result.header[0x0A]);
	result.imageSize  = *(int*)&(result.header[0x22]);
	result.width      = *(int*)&(result.header[0x12]);
	result.height     = *(int*)&(result.header[0x16]);

	if (result.imageSize == 0) {
		 
		result.imageSize = result.width * result.height * 3;
	}

	if (result.dataPos == 0) 
	{ 
		result.dataPos = 54;
	}


	result.data = (unsigned char*) scratch_alloc(result.imageSize);
	
	if (result.data) 
	{
		fread(result.data, 1, result.imageSize, fp);
	}

	fclose(fp);

	return result;
}


struct BMP loadBMP(const char* filename) 
{ 
	struct BMP result;	
	result.dataPos = 0;
	result.width = 0;
	result.height = 0;
	result.imageSize = 0;
	result.data = NULL;

	FILE* fp = fopen(filename, "rb");
	
	if (fp == NULL) 
	{ 
		printf("unable to open file [%s]\n", filename);
		return result;
	}


	if (fread(result.header, 1, 54, fp) != 54) 
	{ 
		printf("invalid bmp format [%s]\n", filename);
		fclose(fp);
		return result;
	}

	if (result.header[0] != 'B' || result.header[1] != 'M') 
	{ 
		printf("invalid bmp format [%s]\n", filename);
		fclose(fp);
		return result;
	}

	// Make sure this is a 24bpp file
	if ( *(int*)&(result.header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    fclose(fp); return result;}
	if ( *(int*)&(result.header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    fclose(fp); return result;}

	result.dataPos    = *(int*)&(result.header[0x0A]);
	result.imageSize  = *(int*)&(result.header[0x22]);
	result.width      = *(int*)&(result.header[0x12]);
	result.height     = *(int*)&(result.header[0x16]);

	if (result.imageSize == 0) {
		 
		result.imageSize = result.width * result.height * 3;
	}

	if (result.dataPos == 0) 
	{ 
		result.dataPos = 54;
	}

	result.data = (unsigned char*) malloc(result.imageSize);
	fread(result.data, 1, result.imageSize, fp);
	fclose(fp);

	return result;
}


void freeBMP(BMP & bmp) 
{
	free(bmp.data);
	bmp.data = NULL;
	bmp.height = 0;
	bmp.width = 0;
	bmp.imageSize = 0;
}

char* readfile(const char* filename) 
{ 
 	FILE* fp = fopen(filename, "rb");
	
	if (!fp) 
	{ 
		printf("unable to open file: [%s]\n", filename);
		return 0;
	}
	
	long lSize;
	char* buffer;
	fseek(fp, 0L, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);
	buffer = (char*) calloc(1, lSize + 1);
	
	if (!buffer) { 
		fclose(fp);
		return 0;
	}

	if (1 != fread(buffer, lSize, 1, fp)) 
	{ 
		fclose(fp);
		free(buffer);
		return 0;
	}

	fclose(fp);
	return buffer;
}

char* readfile_scratch(const char* filename) 
{ 
 	FILE* fp = fopen(filename, "rb");
	
	if (!fp) 
	{ 
		printf("unable to open file: [%s]\n", filename);
		return 0;
	}

	long lSize;
	char* buffer;
	fseek(fp, 0L, SEEK_END);
	lSize = ftell(fp);
	rewind(fp);
	buffer = (char*) scratch_calloc(1, lSize + 1);
	
	if (!buffer) 
	{ 
		fclose(fp);
		return 0;
	}

	if (1 != fread(buffer, lSize, 1, fp)) 
	{ 
		fclose(fp);
		return 0;
	}

	fclose(fp);
	return buffer;
}

bool loadOBJ(const char* fileName, std::vector<glm::vec3> &vertices, std::vector<glm::vec2> &uvs, std::vector<glm::vec3> &normals)
{	
	FILE* fp = fopen(fileName, "rb");
	
	if (fp == NULL) 
	{ 
		printf("unable to open path [%s]\n", fileName);
		return false;
	}

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;
	
	char lineHeader[128];
	
	while (1) 
	{ 
		int result = fscanf(fp, "%s", lineHeader);
		
		if (result == EOF) 
		{ 
			break;
		}

		// read in a single vertex
		if (strcmp(lineHeader, "v") == 0) 
		{ 
			glm::vec3 vertex;
			fscanf(fp, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}

		// read in a single uv
		else if ( strcmp( lineHeader, "vt" ) == 0 )
		{
			glm::vec2 uv;
			fscanf(fp, "%f %f\n", &uv.x, &uv.y );
			temp_uvs.push_back(uv);
		} 

		// read in a single normal
		else if ( strcmp( lineHeader, "vn" ) == 0 )
		{
			glm::vec3 normal;
			fscanf(fp, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}

		// read in a single face
		else if ( strcmp( lineHeader, "f" ) == 0 )
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(fp, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			
			if (matches != 9)
			{
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);	
		}
	}

	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		unsigned int vertexIndex = vertexIndices[i];
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < normalIndices.size(); i++)
	{
		unsigned int normalIndex = normalIndices[i];
		glm::vec3 normal = temp_normals[normalIndex - 1];
		normals.push_back(normal);
	}

	for (unsigned int i = 0; i < uvIndices.size(); i++)
	{
		unsigned int uvindex = uvIndices[i];
		glm::vec2 uv = temp_uvs[uvindex - 1];
		uvs.push_back(uv);
	}

	fclose(fp);
	return true;
}