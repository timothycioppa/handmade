#ifndef _APPLICATIONDATA_HPP
#define _APPLICATIONDATA_HPP

#include "SceneObject.hpp"
#include "Transform.hpp"
#include "TextureStore.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "Camera.hpp"
using namespace std;

class ApplicationData 
{

	public:	
		float version;
		std::vector<SceneObject> SceneObjects;

		ApplicationData(const char* source) 
		{
		
			//LoadAppData(source);
		} 

		void LoadAppData(const char* source) { 
			fstream stream;
			stream.open(source);
			string key;			

			while (true)
			{ 
				stream >> key;

				if (stream.eof()) 
				{ 
					break;
				}
			
				if (key == "version") 
				{ 
					stream >> version;
				} 
				else if (key == "sceneobject") 
				{ 
					string name;
					string soKey;
					string modelSource;
					Transform transform;

					// model obj file
					stream >> soKey >> name;
					stream >> soKey >> modelSource;

					std::cout << "Loading " << name << " " << modelSource << endl;

					// material
					std::string mainTex, shaderType;
					float shininess;
					glm::vec3 specular, diffuse;
					stream >> soKey;
					stream >> soKey >> shaderType;
					stream >> soKey >> mainTex;
					stream >> soKey >> diffuse.r >> diffuse.g >> diffuse.b;
					stream >> soKey >> specular.r >> specular.g >> specular.b;
					stream >> soKey >> shininess;

					MaterialInfo matInfo;
					matInfo.MainTex = mainTex;
					matInfo.ShaderType = shaderType;
					matInfo.specular = specular;
					matInfo.diffuse = diffuse;
					matInfo.shininess = shininess;
								
					// model transform
					stream >> soKey;
					stream >> soKey >> transform.position.x >> transform.position.y >> transform.position.z;
					stream >> soKey >> transform.rotation;
					stream >> soKey >> transform.axis.x >> transform.axis.y >> transform.axis.z;
					stream >> soKey >> transform.scale.x >> transform.scale.y >> transform.scale.z;

					SceneObjects.push_back(SceneObject(
						name,
						modelSource, 
						matInfo, 
						transform));
				}
			} 

			stream.close();
		} 
};


#endif