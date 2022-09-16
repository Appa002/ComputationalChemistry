#include "linerenderer.h"
#include <rendering/globalprogrammemanager.h>

LineRenderer::LineRenderer()
{

}

void LineRenderer::initialise()
{
    std::vector<float> vertecies = {
        0, 0.5, 0,
        1, 0.5, 0,
        1, -0.5, 0,

        0, 0.5, 0,
        0, -0.5, 0,
        1, -0.5, 0
    };

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER,  sizeof(float) * vertecies.size(), &vertecies[0], GL_STATIC_DRAW);
 }

void LineRenderer::draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    return;
    auto programme = GlobalProgrammeManager::get()->programmeAt["line"];

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

void LineRenderer::update(double)
{
    glm::vec3 startPos = {*aPositionObserver[0], *aPositionObserver[1], *aPositionObserver[2]};
    glm::vec3 endPos = {*bPositionObserver[0], *bPositionObserver[1], *bPositionObserver[2]};
    glm::vec3 centre = {startPos[0] + (endPos[0] - startPos[0]) / 2.0f,
                                 startPos[1] + (endPos[1] - startPos[1]) / 2.0f,
                                 startPos[2] + (endPos[2] - startPos[2]) / 2.0f};

    glm::vec3 cross = glm::normalize(glm::cross(glm::normalize(endPos), glm::normalize(startPos)));
    float angle = glm::acos(glm::dot(glm::normalize(endPos), glm::normalize(startPos)));

    m_modelMatrix =
            glm::translate(endPos) *
            glm::rotate(angle, cross) *
            glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));

}

void LineRenderer::setABObserver(std::vector<float *> a, std::vector<float *> b)
{
    aPositionObserver = a;
    bPositionObserver = b;
}
