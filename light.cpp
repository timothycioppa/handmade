#include "light.hpp"
#include  "platform_common.hpp"

void update_light_direction(light & l)
{ 
        glm::vec3 front;
        float yawRad = glm::radians(l.Yaw);
        float pitchRad = glm::radians(l.Pitch);
        front.x = cos(yawRad) * cos(pitchRad);
        front.y = sin(pitchRad);
        front.z = sin(yawRad) * cos(pitchRad);
        l.Forward = glm::normalize(front);
        l.Right = glm::normalize(glm::cross(l.Forward, l.WorldUp));  
        l.Up    = glm::normalize(glm::cross(l.Right, l.Forward));
        l.view = glm::lookAt(l.Position, l.Position + l.Forward, l.Up);
        l.lightSpace = (l.projection) * (l.view);
}

void update_projection_matrix(light & l) 
{ 
    l.projection =    glm::ortho(-0.5f * l.frustrumWidth, 0.5f * l.frustrumWidth, -0.5f * l.frustrumHeight, 0.5f * l.frustrumHeight, l.near, l.far);
    l.lightSpace = l.projection * l.view;
}


