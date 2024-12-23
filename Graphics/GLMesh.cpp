
#include "GLMesh.hpp"


void load_mesh(const char* fileName, static_mesh & m) 
{ 

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    
     if (loadOBJ(fileName, vertices, uvs, normals)) 
    { 
        glGenVertexArrays(1, &(m.VAO));	
        glBindVertexArray(m.VAO);
        m.vertexCount = vertices.size();

        {
            // generate and load vertex data
            glGenBuffers(1,&(m.VBO));
            glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

            // generate and load normal data
            glGenBuffers(1,&(m.NBO));
            glBindBuffer(GL_ARRAY_BUFFER, m.NBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

            //generate and load UV data
            glGenBuffers(1,&(m.UVBO));
            glBindBuffer(GL_ARRAY_BUFFER, m.UVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, 0, (void*) 0);
        }
        
        glBindVertexArray(0);
    }
}

void render_mesh(static_mesh & m) { 
    glBindVertexArray(m.VAO);
    glDrawArrays(GL_TRIANGLES, 0, m.vertexCount);     
    glBindVertexArray(0);
}

void release_mesh (static_mesh & m) { 
    glDeleteVertexArrays(1, &(m.VAO));
}
