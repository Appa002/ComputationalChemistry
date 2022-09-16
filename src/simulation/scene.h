#ifndef SCENE_H
#define SCENE_H

#include <rendering/sphererenderer.h>
#include "mdsimulation.h"
#include <stdexcept>
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <mutex>
#include <vector>
#include <common/atom.h>
#include <unordered_map>


struct OpenGLError : public std::runtime_error
{
    OpenGLError(std::string const & str) : std::runtime_error(str) {}
};

class Scene
{
public:
    Scene();
    ~Scene();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void update(double deltaTime);
    void wheelEvent(int delta);
    void mouseLeftDrag(int xDelta, int yDelta);
    void mouseRightDrag(int xDelta, int yDelta);
    void resetCamera();

    void dropMolecule();


    bool m_temprun = false;
    bool m_tempSingleStep = false;

    void loadMolecule(Atom* molecule);

private:
    std::mutex changingMoleculeMutex;

    std::vector<float> m_masses;
    std::vector<float> m_charges;
    std::vector<float> m_positionsX;
    std::vector<float> m_positionsY;
    std::vector<float> m_positionsZ;

    std::vector<float> m_bonds;
    std::vector<float> m_angleBend;
    std::vector<float> m_torsionBend;

    void flattenMoleculeStructure(Atom* molecule);
    void putOnHat();

    std::vector<Atom> atoms;

    MDSimulation* m_simulation;


    float m_cameraR = 30.0f;
    float m_cameraAngleIncline = 2.01f;
    float m_cameraAngleAttitude = 0.77f;
    glm::vec3 m_cameraOffset = {0.0f, 0.0f, 0.0f};

    std::vector<IRenderer*> m_renderer;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    void updateViewMatrix();
};

#endif // SCENE_H
