#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "platform_common.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

class Transform { 
	private :
		glm::mat4 localToWorld;
		bool dirty;

		
	public:
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 axis;
		float rotation;

		void init(glm::vec3 pos, glm::vec3 s, float angle, glm::vec3 axs) { 
			dirty = true;
			position = pos;
			scale = s;
			axis = axs;
			rotation = angle;
			localToWorld = localToWorldMatrix();
		}
		
		Transform() { 
			init(glm::vec3(0.0f), glm::vec3(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		}

		Transform (const Transform & other) 
		{
			init(other.position, other.scale, other.rotation, other.axis);
		}

		Transform(glm::vec3 pos, glm::vec3 s, float angle, glm::vec3 axs) 
		{
			init(pos, s, angle, axs);
		}

		void SetPosition(glm::vec3 amount) 
		{ 
			position = amount;
			dirty = true;
		}

		void SetAngle(float amount) 
		{ 
			rotation = amount;
			dirty = true;
		}

		
		void SetAxis(glm::vec3 amount) 
		{ 
			axis = amount;
			dirty = true;
		}

		void SetScale(glm::vec3 ns) 
		{ 
			scale = ns;
			dirty = true;
		}

		void MarkDirty() { 
			dirty = true;
		}

		glm::mat4 localToWorldMatrix() 
		{ 
			if (dirty) 
			{ 
				dirty = false;
				glm::mat4 _s = glm::scale(glm::mat4(1.0f), scale);
				glm::mat4 _r = glm::rotate(glm::mat4(1.0f), rotation, axis);
				glm::mat4 _t = glm::translate(glm::mat4(1.0f), position);
				localToWorld = _t * _r * _s;
			}

			return localToWorld;
		}
};

#endif