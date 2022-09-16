#include "gridrenderer.h"
#include <vector>
#include "globalprogrammemanager.h"

GridRenderer::GridRenderer()
{

}

void GridRenderer::initialise()
{
    std::vector<float> vertecies = {
        -1, 1, 0,
        1, 1, 0,
        1, -1, 0,

        -1, 1, 0,
        -1, -1, 0,
        1, -1, 0
    };

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,  sizeof(float) * vertecies.size(), &vertecies[0], GL_STATIC_DRAW);

    m_modelMatrix =
            glm::translate(glm::vec3(0, 0, 0)) *
            glm::rotate(glm::pi<float>()/2.0f, glm::vec3(1.0f, 0.0f, 0.0f)) *
            glm::scale(glm::vec3(100.0f, 100.0f, 100.0f));
}

void GridRenderer::draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    auto programme = GlobalProgrammeManager::get()->programmeAt["grid"];

    // Use main programme
    glUseProgram(programme.id);

    // Setup shader attributes....

    // ..... Vertex position .....
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);


    glm::mat4 mvp = projectionMatrix * viewMatrix * m_modelMatrix;
    glUniformMatrix4fv(programme.uniforms["mvp"], 1, GL_FALSE, &mvp[0][0]);

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(0);
}

void GridRenderer::update(double)
{

}
