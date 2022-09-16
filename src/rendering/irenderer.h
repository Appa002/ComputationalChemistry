#ifndef IRENDERER_H
#define IRENDERER_H

#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>

class IRenderer
{
public:
    IRenderer() = default;
    virtual ~IRenderer() = default;
    virtual void initialise() = 0;
    virtual void draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)  = 0;
    virtual void update(double deltaTime) = 0;
};

#endif // IRENDERER_H
