#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include "irenderer.h"

class GridRenderer : public IRenderer
{
public:
    GridRenderer();

    virtual void initialise() override;
    virtual void draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) override;
    virtual void update(double deltaTime) override;

private:
    GLuint m_vertexBuffer;
    glm::mat4 m_modelMatrix;
};

#endif // GRIDRENDERER_H
