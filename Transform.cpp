#include "Transform.hpp"

void initialize_transform(Transform & t, glm::vec3 pos, glm::vec3 s, float angle, glm::vec3 axs) 
{
    t.position = pos;
    t.scale = s;
    t.axis = axs;
    t.rotation = angle;
    update_model_matrix(t);
}

void update_model_matrix(Transform & t) 
{ 
    glm::mat4 _s = glm::scale(glm::mat4(1.0f), t.scale);
    glm::mat4 _r = glm::rotate(glm::mat4(1.0f), t.rotation, t.axis);
    glm::mat4 _t = glm::translate(glm::mat4(1.0f), t.position);
    t.localToWorld = _t * _r * _s;
}
