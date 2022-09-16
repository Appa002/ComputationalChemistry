#include "scene.h"
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <iostream>
#include <cmath>
#include <queue>

#include <functional>

#include <rendering/linerenderer.h>
#include <rendering/gridrenderer.h>

#include <common/parameterization.h>

#define COLOUR_PALLET_RED 1.0f, 0.063f, 0.325f
#define COLOUR_PALLET_YELLOW 1.0f, 1.0f, 0.51f
#define COLOUR_PALLET_DARK 0.012f, 0.048f, 0.149f
#define COLOUR_PALLET_BEACH 0.953f, 0.076f, 0.824f
#define COLOUR_PALLET_WHITE 1.0f, 1.0f, 1.0f

/*
                3,
                {2.30f, 0.712f, 0.712f},
                {1106.523703834847f, 1106.523703834847f, -1.0f}, //kij ; kcal/mol
                {0.4612590912601745f, 0.4612590912601745f, -1.0f}, // rij ; A
                {2.0f, 2.0f, -1.0f}, // D = 70 kcal/mol
                { 15.999f, 1.00784f, 1.00784f }, // masses U
                { {0.0f, 0.0f, 0.0f},{0.49f, -0.106f, 0.0f}, {-0.49f, -0.106f, 0.0f}},
                {
                    1,
                    670.5723161573269f,
                    1.823869068f,
                    0,
                    3.0f,
                    0.30016229106529296f,
                    0.26712618322978676f,
                    0.266720763688431f,
                    1,
                    2
                }

*/

Scene::Scene()
{

//-0.094 205 6 = rbo
    // 0 = ren

}

Scene::~Scene()
{
    m_simulation->cleanUp();
    delete m_simulation;

    for(auto const& renderer : m_renderer)
        delete renderer;
}

void Scene::initializeGL()
{
    glewInit();
    // Setup Simulation
    m_simulation = new MDSimulation;


    // Set up the rendering context, load shaders and other resources, etc.:
    glClearColor(0.011f, 0.098f, 0.149f, 1.0f);

    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS);


    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_renderer.push_back(new GridRenderer());
    m_renderer[0]->initialise();
    updateViewMatrix();

    return;

}

void Scene::resizeGL(int w, int h)
{
    // Update projection matrix and other size related settings:
    float fov = 80.0f;
    if (w/h < 1)
        fov *= 1.5f;

    m_projectionMatrix = glm::perspective(
        glm::radians(fov), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
        float(w) / float(h),       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
        0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        100.0f             // Far clipping plane. Keep as little as possible.
    );

    //glUniform2f(m_windowSizeHandle, float(w), float(h));
}

void Scene::paintGL()
{
    // Draw the scene:
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(auto itr = m_renderer.rbegin(); itr != m_renderer.rend(); ++itr){
       (*itr)->draw(m_viewMatrix, m_projectionMatrix);
    }
}

void Scene::update(double deltaTime)
{
    changingMoleculeMutex.lock();
    if(m_temprun){
        m_simulation->step(deltaTime);
        if(m_tempSingleStep){
            m_temprun = false;
            m_tempSingleStep = false;
        }
    }

    for(auto& renderer : m_renderer){
        renderer->update(deltaTime);
    }
    changingMoleculeMutex.unlock();
}

void Scene::wheelEvent(int delta)
{
    m_cameraR += delta * 0.01f;

    updateViewMatrix();
}

void Scene::mouseLeftDrag(int xDelta, int yDelta)
{
    const float PI = 3.14159265359f;
    const float MOUSE_SENSE = 0.005f;

    m_cameraAngleAttitude += xDelta * MOUSE_SENSE;
    float inclineD =  yDelta * MOUSE_SENSE;

    if(m_cameraAngleAttitude > 2*PI)
        m_cameraAngleAttitude -= 2*PI;
    else if (m_cameraAngleAttitude < 0)
        m_cameraAngleAttitude += 2*PI;

    if(m_cameraAngleIncline + inclineD >= PI)
        m_cameraAngleIncline = PI - 0.005f;
    else if (m_cameraAngleIncline + inclineD <= 0)
        m_cameraAngleIncline = 0.005f;
    else
        m_cameraAngleIncline += inclineD;

    updateViewMatrix();

}

void Scene::mouseRightDrag(int xDelta, int yDelta)
{
    const float MOUSE_SENSE = 0.015f;
    float z = std::cos(m_cameraAngleAttitude)*  std::sin(m_cameraAngleIncline) * m_cameraR;
    float x = std::sin(m_cameraAngleAttitude) * std::sin(m_cameraAngleIncline) * m_cameraR;
    float y = std::cos(m_cameraAngleIncline) * m_cameraR;

    glm::vec3 forward (-x, -y, -z);
    forward = glm::normalize(forward);

    glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));
    glm::vec3 up = glm::cross(forward, right);

    right = glm::normalize(right);
    up = glm::normalize(up);

    m_cameraOffset += up * static_cast<float>(yDelta) * MOUSE_SENSE;
    m_cameraOffset += right * static_cast<float>(xDelta) * MOUSE_SENSE;
    updateViewMatrix();

}

void Scene::resetCamera()
{
    m_cameraR = 30.0f;
    m_cameraAngleIncline = 2.01f;
    m_cameraAngleAttitude = 0.77f;
    m_cameraOffset = {0.0f, 0.0f, 0.0f};
    updateViewMatrix();
}

void Scene::dropMolecule()
{
    m_simulation->cleanUp();
}

void Scene::loadMolecule(Atom *molecule)
{
    changingMoleculeMutex.lock();

    m_masses.clear();
    m_charges.clear();
    m_positionsX.clear();
    m_positionsY.clear();
    m_positionsZ.clear();

    m_bonds.clear();
    m_angleBend.clear();
    m_torsionBend.clear();

    flattenMoleculeStructure(molecule);
    unsigned numAtoms = static_cast<unsigned>(m_masses.size());
    unsigned numBonds = static_cast<unsigned>(m_bonds.size() / 5);
    unsigned numValence = static_cast<unsigned>(m_angleBend.size()/9);
    unsigned numTorsions = static_cast<unsigned>(m_torsionBend.size()/7);

    putOnHat();

    m_simulation->initialise(numAtoms, m_charges, m_bonds,
                             numBonds, m_masses, {m_positionsX, m_positionsY, m_positionsZ},
                             m_angleBend, numValence, m_torsionBend, numTorsions);

    // Add all the required spehere renderer...

    while(m_renderer.begin() + 1 != m_renderer.end()){// clear all besides grid renderer
        delete *m_renderer.rbegin();
        m_renderer.erase(m_renderer.end() - 1);
    }

    auto colourMap = molecule->display_colourMap;

    // goes through all atoms and assigns them a sphere renderer...
    std::function<void(Atom*)> dfs = [&](Atom* atom) -> void{

        SphereRenderer* sr = new SphereRenderer(colourMap[atom->name]);
        sr->initialise();
        sr->setPositionObserver(m_simulation->getPositionObservers(atom->idx));

        m_renderer.push_back(sr);

        for(auto const & child : atom->children)
            dfs(child);
    };

    dfs(molecule);
    changingMoleculeMutex.unlock();

}

void Scene::flattenMoleculeStructure(Atom* molecule)
{
    const size_t offset = this->m_charges.size();

    m_masses.push_back(molecule->mass);
    m_charges.push_back(molecule->partialCharge);
    m_positionsX.push_back(molecule->x);
    m_positionsY.push_back(molecule->y);
    m_positionsZ.push_back(molecule->z);

    molecule->idx = 0;

    // Bonds, position, mass, and charge...
    std::function<void(size_t, size_t, Atom*)> bpmc = [&](size_t callingIdx, size_t nth, Atom* atom) -> void{
        m_masses.push_back(atom->mass);
        m_charges.push_back(atom->partialCharge);
        m_positionsX.push_back(atom->x);
        m_positionsY.push_back(atom->y);
        m_positionsZ.push_back(atom->z);

        atom->idx = m_masses.size() - 1;


        m_bonds.push_back(Parameterization::kBond(atom->parent, atom)); //k
        m_bonds.push_back(Parameterization::r(atom->parent, atom)); //r0
        m_bonds.push_back(Parameterization::bondOrder(atom->parent, atom)); //D

        m_bonds.push_back(callingIdx); // atom one
        m_bonds.push_back(atom->idx); // atom two

        // ...
        size_t i = 0;
        for (auto const & child : atom->children){
            bpmc(atom->idx, i++, child);
        }

    };

    {
    size_t i = 0;
    for(auto const & child : molecule->children)
        bpmc(0, i++, child);
    }

    // Valenz angle bending...
    std::function<void(Atom*)> angle = [&](Atom* atom) -> void{
        if(atom->parent) {
            for(auto const & child : atom->children){
                auto const valenz = Parameterization::fourierValenz(atom->parent, atom, child);

                m_angleBend.push_back(Parameterization::kValenz(atom->parent, atom, child)); // kijk
                m_angleBend.push_back(atom->valenzAngle); // natrual angle
                m_angleBend.push_back(atom->idx); // atom centre
                m_angleBend.push_back(3); // # fourier
                m_angleBend.push_back(valenz[0]); // fourier ...
                m_angleBend.push_back(valenz[1]);
                m_angleBend.push_back(valenz[2]);
                m_angleBend.push_back(atom->parent->idx); // atom one
                m_angleBend.push_back(child->idx); // atom two
            }
        }

        if(atom->children.size() >= 2){
            for (size_t i = 0; i < atom->children.size(); i += 2){
                auto const valenz = Parameterization::fourierValenz(atom->children[i], atom, atom->children[i+1]);

                m_angleBend.push_back(Parameterization::kValenz(atom->children[i], atom, atom->children[i+1])); // kijk
                m_angleBend.push_back(atom->valenzAngle); // natrual angle
                m_angleBend.push_back(atom->idx); // atom centre
                m_angleBend.push_back(3); // # fourier
                m_angleBend.push_back(valenz[0]); // fourier ...
                m_angleBend.push_back(valenz[1]);
                m_angleBend.push_back(valenz[2]);
                m_angleBend.push_back(atom->children[i]->idx); // atom one
                m_angleBend.push_back(atom->children[i+1]->idx); // atom two
            }
        }

        for(auto const & child : atom->children){
            angle(child);
        }

    };

    angle(molecule);

    std::function<void (std::vector<Atom*>)> handleChain = [&](std::vector<Atom*> chain) -> void {
        if(chain.size() < 4)
            return;

        auto subChain = std::vector<Atom*>(chain.begin(), chain.begin() + 4);
        // [
        //      descriptor_idx_0,
        //      descriptor_idx_1,
        //      ...
        //      descriptor_idx_n,
        //      {
        //          V,
        //          n,
        //          firstExpansion,
        //          atom_inner_one,
        //          atom_inner_two,
        //          atom_outer_one,
        //          atom_outer_two
        //      }

        m_torsionBend.push_back(subChain[3]->torsionalV); // V
        m_torsionBend.push_back(subChain[3]->torsionalN); // n
        m_torsionBend.push_back(Parameterization::torsionalFirstExpansion(subChain[3])); // firstExpansion
        m_torsionBend.push_back(subChain[1]->idx); // atom_inner_one
        m_torsionBend.push_back(subChain[2]->idx); // atom_inner_two
        m_torsionBend.push_back(subChain[0]->idx); // atom_outer_one
        m_torsionBend.push_back(subChain[3]->idx); // atom_outer_two

        handleChain(std::vector<Atom*>(chain.begin() + 1, chain.end()));

    };

    std::vector<std::vector<Atom*>> chains;
    molecule->allChains(chains);
    for(auto const & chain : chains){
        handleChain(chain);
    }


}

void Scene::putOnHat()
{
    size_t bondN = m_bonds.size() / 5;
    for(size_t i = 0; i < bondN; i++){
        size_t idx = i * 5 + bondN;
        m_bonds.insert(m_bonds.begin() + i, static_cast<float>(idx));
    }

    size_t valenzN = m_angleBend.size() / 9;
    for(size_t i = 0; i < valenzN; i++){
        size_t idx = i * 9 + valenzN;
        m_angleBend.insert(m_angleBend.begin() + i, static_cast<float>(idx));
    }

    size_t torsionN = m_torsionBend.size() / 7;
    for(size_t i = 0; i < torsionN; i++){
        size_t idx = i * 7 + torsionN;
        m_torsionBend.insert(m_torsionBend.begin() + i, static_cast<float>(idx));
    }
}

void Scene::updateViewMatrix()
{
    float z = std::cos(m_cameraAngleAttitude)*  std::sin(m_cameraAngleIncline) * m_cameraR;
    float x = std::sin(m_cameraAngleAttitude) * std::sin(m_cameraAngleIncline) * m_cameraR;
    float y = std::cos(m_cameraAngleIncline) * m_cameraR;

    //z += m_cameraOffset[2];
    //x += m_cameraOffset[0];
    //y += m_cameraOffset[1];

    glm::vec3 forward (-x, -y, -z);
    forward = glm::normalize(forward);

    glm::vec3 right = glm::cross(forward, glm::vec3(0, 1, 0));
    glm::vec3 up = glm::cross(forward, right);

    m_viewMatrix = glm::lookAt(
        glm::vec3(x,y,z), // the position of your camera, in world space
        glm::vec3(0, 0, 0),   // where you want to look at, in world space
        up
    );

    m_viewMatrix = glm::translate(m_viewMatrix, m_cameraOffset);

}
