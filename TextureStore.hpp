#ifndef TEXTURE_STORE_HPP
#define TEXTURE_STORE_HPP

#include <string>
#include <map>
#include "Graphics/GLTexture.hpp"

class TextureStore 
{ 
    public:
        std::map<std::string, texture_info> TextureMap; 

        void LoadTexture(std::string source) 
        { 
            if (TextureMap.count(source) == 0) 
            { 
                texture_info info;
                TextureMap.insert({source, info});
                texture_info& tex = TextureMap[source];
                load_texture(source.c_str(), tex);                     
            }        
        }

        texture_info* GetTexture(std::string source) 
        { 
            if (TextureMap.count(source) == 0) 
            { 
                LoadTexture(source);              
            }   
            return &TextureMap[source]; 
        }    
};

#endif