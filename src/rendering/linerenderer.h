#ifndef LINERENDERER_H
#define LINERENDERER_H

#include "irenderer.h"
#include <vector>

class LineRenderer : public IRenderer
{
public:
    LineRenderer();

    virtual void initialise();
    virtual void draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    virtual void update(double deltaTime);

    void setABObserver(std::vector<float*> a, std::vector<float*> b);

private:
    std::vector<float*> aPositionObserver = {nullptr, nullptr, nullptr};
    std::vector<float*> bPositionObserver = {nullptr, nullptr, nullptr};

    GLuint m_vertexBuffer;

    glm::mat4 m_modelMatrix;
};

#endif // LINERENDERER_H
