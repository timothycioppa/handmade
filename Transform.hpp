#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "platform_common.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

struct Transform 
{ 
		glm::vec3 position;
		glm::vec3 scale;
		glm::vec3 axis;
		float rotation;
		glm::mat4 localToWorld;
};

void initialize_transform(Transform & t, glm::vec3 pos, glm::vec3 s, float angle, glm::vec3 axs);
void update_model_matrix(Transform & t);
glm::mat4 localToWorld(Transform & t);

#endif