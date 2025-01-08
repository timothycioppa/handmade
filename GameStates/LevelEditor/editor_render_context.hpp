#ifndef _EDITOR_RENDER_CONTEXT
#define _EDITOR_RENDER_CONTEXT

#include "../../platform_common.hpp"

struct editor_render_context
{
    glm::mat4 clipToWorld;
    glm::vec3 cameraPosition;
};

#endif