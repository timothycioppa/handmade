#ifndef _SCENEOBJECT_HPP
#define _SCENEOBJECT_HPP

#include "Transform.hpp"
#include <string>
#include "Graphics/GLMesh.hpp"
#include <iostream>
#include "TextureStore.hpp"
#include "Material.hpp"

struct MaterialInfo 
{ 
    std::string MainTex;
    std::string ShaderType;
    glm::vec3 specular;
    glm::vec3 diffuse;
    float shininess;
};

class SceneObject 
{ 
    private:
        MaterialInfo matinfo;

    public:
        bool enabled;
        Transform transform;
        static_mesh mesh;
        Material material;
        std::string Name;

        SceneObject(
            std::string name,
            std::string modelSource, 
            MaterialInfo matInfo, 
            const Transform & other) 
        { 
            enabled = true;
            Name = name;
            transform = other;      
            load_mesh(modelSource.c_str(), mesh);
            this->matinfo = matInfo;
        } 
 
        void Release() 
        { 
            release_mesh(mesh);
        }
};

#endif