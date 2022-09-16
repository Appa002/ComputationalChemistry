#include "sphererenderer.h"
#include "globalprogrammemanager.h"
#include <vector>
#include <iostream>

SphereRenderer::SphereRenderer(std::vector<float> colour)
{
    this->m_colour = colour;
}

void SphereRenderer::initialise()
{
    // Create Sphere vertices ....
    std::vector<float> vertices;

    const float PI = 3.14159265359f;
    const float sectorCount = 36;
    const float stackCount = 18;
    const float radius = 1.0f;


    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);             // r * cos(u)
        float z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            float x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            float y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

        }

    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if(i != 0)
            {
                m_indexData.push_back(k1);
                m_indexData.push_back(k2);
                m_indexData.push_back(k1+1);   // k1---k2---k1+1
            }

            if(i != (stackCount-1))
            {
                m_indexData.push_back(k1 + 1);
                m_indexData.push_back(k2);
                m_indexData.push_back(k2+1);
            }
        }
    }

    std::vector<float> interleavedVertices;
    std::size_t i, j;
    std::size_t count = vertices.size();
    for(i = 0, j = 0; i < count; i += 3, j += 2)
    {
        interleavedVertices.push_back(vertices[i]);
        interleavedVertices.push_back(vertices[i+1]);
        interleavedVertices.push_back(vertices[i+2]);
    }


    // ... created sphere vertices

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * interleavedVertices.size(), &interleavedVertices[0], GL_STATIC_DRAW);


    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * m_indexData.size(), &m_indexData[0], GL_STATIC_DRAW);

    this->transform(0, 0, 0);
}

void SphereRenderer::setPositionObserver(float *x, float *y, float *z)
{
    m_positionFromMDSimulation[0] = x;
    m_positionFromMDSimulation[1] = y;
    m_positionFromMDSimulation[2] = z;
}

void SphereRenderer::setPositionObserver(std::vector<float *> vec)
{
    m_positionFromMDSimulation = vec;
}

void SphereRenderer::draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{

    auto programme = GlobalProgrammeManager::get()->programmeAt["sphere"];

    // Use main programme
    glUseProgram(programme.id);

    // Setup shader attributes....

    // ..... Vertex position .....
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer); // 32    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // .... MVP Matrix ....
    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * m_modelMatrix;
    glUniformMatrix4fv(programme.uniforms["mvp"], 1, GL_FALSE, &mvpMatrix[0][0]);

    // ... Colour ...
    glUniform3f(programme.uniforms["atomColour"], m_colour[0], m_colour[1], m_colour[2]);
    glEnable(GL_PRIMITIVE_RESTART);

    // Draw
    glDrawElements(GL_TRIANGLES, m_indexData.size(), GL_UNSIGNED_INT,&m_indexData[0]);


    glDisable(GL_PRIMITIVE_RESTART);

    glDisableVertexAttribArray(0);

}

void SphereRenderer::update(double)
{
    if(m_positionFromMDSimulation[0] != nullptr)
        transform(*m_positionFromMDSimulation[0], *m_positionFromMDSimulation[1], *m_positionFromMDSimulation[2]);
}

std::vector<float> SphereRenderer::transform()
{
    return this->m_transform;
}

std::vector<float> SphereRenderer::transform(float x, float y, float z)
{
    // Initial Matrix Setup...
    m_modelMatrix =
            glm::translate(glm::vec3(x, y, z)) *
            glm::rotate( 32.0f, glm::vec3(1, 0, 1) ) *
            glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

    m_transform = {x, y, z};
    return m_transform;
}

std::vector<float> SphereRenderer::transform(std::vector<float> in)
{
    // Initial Matrix Setup...

    m_modelMatrix =
            glm::translate(glm::vec3(in[0], in[1], in[2])) *
            glm::rotate( 32.0f, glm::vec3(1, 0, 1) ) *
            glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));

    m_transform = in;
    return m_transform;
}
