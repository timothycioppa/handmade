#include "TextureStore.hpp"
#include <map>

std::map<std::string, texture_info> TextureMap; 

void load_texture(std::string source) 
{ 
    if (TextureMap.count(source) == 0) 
    { 
        texture_info info;
        TextureMap.insert({source, info});
        texture_info& tex = TextureMap[source];
        load_texture(source.c_str(), tex);                     
    }        
}

texture_info* get_texture(std::string source) 
{ 
    if (TextureMap.count(source) == 0) 
    { 
        load_texture(source);              
    }   
    return &TextureMap[source]; 
}    

void release_texture_store() 
{ 
    for (auto & [key, value] : TextureMap ) 
	{ 
		release_texture(value);
	}
}