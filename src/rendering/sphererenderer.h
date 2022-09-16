#ifndef SPHERERENDERER_H
#define SPHERERENDERER_H

#include <stdexcept>
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include "irenderer.h"

using observer_float = float*;

class SphereRenderer : public IRenderer
{
public:
    SphereRenderer(std::vector<float> m_colour = {1.0f, 1.0f, 0.51f});

    virtual void initialise() override;

    void setPositionObserver(float* x, float* y, float* z);
    void setPositionObserver(std::vector<float*> vec);

    virtual void draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) override;

    virtual void update(double deltaTime) override;

    std::vector<float> transform();
    std::vector<float> transform(float x, float y, float z);
    std::vector<float> transform(std::vector<float> in);

private:
    std::vector<float> m_colour = {1.0f, 1.0f, 0.51f};
    std::vector<observer_float> m_positionFromMDSimulation = {nullptr, nullptr, nullptr};

    glm::mat4 m_modelMatrix;

    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;

    std::vector<int> m_indexData;

    std::vector<float> m_transform;
};

#endif // SPHERERENDERER_H
