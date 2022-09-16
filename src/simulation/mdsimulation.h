#ifndef MDSIMULATION_H
#define MDSIMULATION_H

#include <vector>
#include <stdexcept>
#include <CL/opencl.h>

struct MDSimulationError : public std::runtime_error
{
    MDSimulationError(std::string str)
        : std::runtime_error(std::move(str)) {}
};

struct MDState{
    // Setup [x0,x1,x2,...,xn, y0,y1,y2,...,yn, z0,z1,z2,...,zn] for cach and transfer optimisation reasons
    // To derefrence x,y,z of i := {array[i], array[i+n], array[i+2*n]}
    // Size: n*3; where n number of atoms
    float* positions = nullptr;

    float* charges = nullptr;
    float* masses = nullptr;


    // Contains the bond information
    // Size: variable
    // [
    //      descriptor_idx_0,
    //      descriptor_idx_1,
    //      ...
    //      descriptor_idx_n,
    //      {
    //          K,
    //          r0,
    //          D,
    //          atom_one,
    //          atom_two
    //      }
    // ]
    float* bonds = nullptr;
    size_t bondsSize = 0;
    unsigned bondsN = 0;

    // Contains the torsion angle information
    // Size: variable
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
    // ]
    float* torsions = nullptr;
    size_t torsionsSize = 0;
    unsigned torsionsN = 0;

    // Contains all angles to which may undergo valence angle bending
    // Size: variable (see setup)
    // [
    //     index_to_descriptor_for_this_angle_in_this_array_0,
    //     index_to_descriptor_for_this_angle_in_this_array_1,
    //     ...
    //     index_to_descriptor_for_this_angle_in_this_array_n,
    //
    //     {
    //     kijk,
    //     natural angle,
    //     atom_centre,
    //     number of fourier expansion terms,
    //     c0
    //     ...
    //     cn
    //     idx of atom one affected
    //     idx of atom two affected
    //     } ; this repeating according to the number of entries above.
    // ]
    float* angles = nullptr;
    size_t angleSize = 0;
    unsigned anglesN = 0;


    unsigned atomN = 0;
};

class MDSimulation
{
public:
    MDSimulation();
    ~MDSimulation();

    void initialise(unsigned numberAtoms, std::vector<float> charges, std::vector<float> bonds, unsigned bondsN,
                    std::vector<float> masses, std::vector<std::vector<float>> positions,
                    std::vector<float> valenceAngles, unsigned anglesN, std::vector<float> torsions,unsigned torsionsN);
    void cleanUp();
    void step(float deltaTime);

    std::vector<float*> getPositionObservers(unsigned i);


private:
    bool curInputBuff = true; // true == alpha

    void setupBondStretchKernel(float deltaTime);
    void setupAngleBendKernel(float deltaTime);
    void setupTorsionBendKernel(float deltaTime);

    //float* m_outPositionBuffer = nullptr;

    void initialiseOpenCL();
    void initialiseOpenCLMemoryObjs();
    void instCopyDataState2GPU();

    cl_mem m_outPositionsObj;
    cl_mem m_positionsObj;
    cl_mem m_chargesObj;
    cl_mem m_bondsObj;
    cl_mem m_massesObj;
    cl_mem m_anglesObj;
    cl_mem m_torsionsObj;

    cl_kernel m_bondStretchKernel;
    cl_kernel m_angleBendKernel;
    cl_kernel m_torsionBendKernel;

    MDState m_state;
    bool m_isInitialised = false;

    cl_context m_context;
    cl_command_queue m_commandQueue;
    cl_program m_program;
    unsigned int m_max_working_group_size;

};

#endif // MDSIMULATION_H
