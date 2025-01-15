#ifndef TEXTURE_STORE_HPP
#define TEXTURE_STORE_HPP

#include <string>
#include "Graphics/GLTexture.hpp"

void load_texture(std::string source) ;
texture_info* get_texture(std::string source) ;
void release_texture_store();

#endif