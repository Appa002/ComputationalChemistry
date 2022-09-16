#include "mdsimulation.h"
#include <CL/opencl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

template <typename T, std::size_t N>
void print_array(const T(&a)[N], std::ostream& o = std::cout)
{
  o << "{";
  for (std::size_t i = 0; i < N-1; ++i)
  {
    o << a[i] << ", ";
  }
  o << a[N-1] << "}\n";
}

template <typename T>
void print_array(T* ptr, std::size_t N, std::ostream& o = std::cout)
{
  o << "{";
  for (std::size_t i = 0; i < N-1; ++i)
  {
    o << ptr[i] << ", ";
  }
  o << ptr[N-1] << "}";
  o << std::endl;
}


MDSimulation::MDSimulation()
{

}

MDSimulation::~MDSimulation()
{

}

void MDSimulation::initialise(
        unsigned numberAtoms,
        std::vector<float> charges,
        std::vector<float> bonds,
        unsigned bondsN,
         std::vector<float> masses,
        std::vector<std::vector<float>> positions,
        std::vector<float> valenceAngles,
        unsigned anglesN,
        std::vector<float> torsions,
        unsigned torsionsN
        )
{
    if(m_isInitialised)
        this->cleanUp();

    m_state.atomN = numberAtoms;
    m_state.charges = new float[numberAtoms];
    m_state.masses = new float[numberAtoms];
    m_state.positions = new float[numberAtoms*3];

    m_state.angles = new float [valenceAngles.size()];
    m_state.angleSize = valenceAngles.size();
    m_state.anglesN = static_cast<unsigned>(anglesN);

    m_state.bonds = new float[bonds.size()];
    m_state.bondsN = bondsN;
    m_state.bondsSize = bonds.size();

    m_state.torsions = new float[torsions.size()];
    m_state.torsionsN = torsionsN;

    //m_outPositionBuffer = new float[numberAtoms*3];



    for(size_t i = 0; i < numberAtoms; i++){
        m_state.charges[i] = charges[i];
        m_state.masses[i] = masses[i];

        m_state.positions[i] = positions[0][i]; //x
        m_state.positions[i + numberAtoms] = positions[1][i]; //y
        m_state.positions[i + 2*numberAtoms] = positions[2][i]; //z
    }

    for(size_t i = 0; i < valenceAngles.size(); i++){
        m_state.angles[i] = valenceAngles[i];
    }

    for(size_t i = 0; i < bonds.size(); i++){
        m_state.bonds[i] = bonds[i];
    }

    for(size_t i = 0; i < torsions.size(); i++){
        m_state.torsions[i] = torsions[i];
    }

    initialiseOpenCL();
    initialiseOpenCLMemoryObjs();
    instCopyDataState2GPU();

    clFinish(m_commandQueue);

    m_isInitialised = true;
}

void MDSimulation::cleanUp()
{
    if(!m_isInitialised)
        return;

    m_state.atomN = 0;
    m_state.anglesN = 0;
    m_state.angleSize = 0;
    m_state.bondsN = 0;
    m_state.bondsSize = 0;
    m_state.torsionsN = 0;
    m_state.torsionsSize = 0;

    delete[] m_state.masses;
    m_state.masses = nullptr;

    delete[] m_state.charges;
    m_state.charges = nullptr;

    delete[] m_state.positions;
    m_state.positions = nullptr;


    delete[] m_state.bonds;
    m_state.bonds = nullptr;

    delete[] m_state.torsions;
    m_state.torsions = nullptr;

    /*delete[] m_outPositionBuffer;
    m_outPositionBuffer = nullptr;
*/
    delete[] m_state.angles;
    m_state.angles = nullptr;

    // OpenCL ...
    int ret = 0;

    ret = clFlush(m_commandQueue);
    ret = clFinish(m_commandQueue);

    clReleaseProgram(m_program);
    clReleaseContext(m_context);
    clReleaseCommandQueue(m_commandQueue);

    ret = clReleaseKernel(m_bondStretchKernel);

    ret = clReleaseMemObject(m_positionsObj);
    ret = clReleaseMemObject(m_chargesObj);
    ret = clReleaseMemObject(m_bondsObj);
    ret = clReleaseMemObject(m_outPositionsObj);
    ret = clReleaseMemObject(m_massesObj);
    ret = clReleaseMemObject(m_torsionsObj);


    m_isInitialised = false;
}

void MDSimulation::step(float deltaTime)
{
    if(!m_isInitialised)
        return;

    int ret = 0;

    instCopyDataState2GPU(); // Put command to copy on gpu

    if(m_state.atomN > std::sqrt(m_max_working_group_size)) // TODO: create a proper warning
        std::cout << "[WARNING] The number of atoms in your simulation is greater than the maximum ("<< std::sqrt(m_max_working_group_size) <<") that can be compute simultaneously." << std::endl;

    /* Bond stretch... */
    setupBondStretchKernel(deltaTime);
    size_t global_item_size[1] = {m_state.bondsN};
    ret = clEnqueueNDRangeKernel(m_commandQueue, m_bondStretchKernel, 1, nullptr,
                                 global_item_size, nullptr, 0, nullptr, nullptr); // We're leting opencl figure out the local group size; I wonder if there is much of a performance impact...


    /* Angle bend... */
    setupAngleBendKernel(deltaTime);
    size_t globalItemSizeAngleBend[1] = {m_state.anglesN};
    ret = clEnqueueNDRangeKernel(m_commandQueue, m_angleBendKernel, 1, nullptr,
                                 globalItemSizeAngleBend, nullptr, 0, nullptr, nullptr); // We're leting opencl figure out the local group size; I wonder if there is much of a performance impact...

    /* Torsion bend... */
    setupAngleBendKernel(deltaTime);
    size_t globalItemSizeTorsionBend[1] = {m_state.torsionsN};
    ret = clEnqueueNDRangeKernel(m_commandQueue, m_torsionBendKernel, 1, nullptr,
                                 globalItemSizeTorsionBend, nullptr, 0, nullptr, nullptr); // We're leting opencl figure out the local group size; I wonder if there is much of a performance impact...


    /* Download positions from GPU... */
    ret = clEnqueueReadBuffer(m_commandQueue, m_outPositionsObj, CL_TRUE, 0, sizeof(float) * m_state.atomN * 3, m_state.positions, 0, nullptr, nullptr);

    // Update positions
    //memcpy(m_state.positions, m_outPositionBuffer, sizeof(float) * 3 * m_state.atomN);


//    print_array(m_outPositionBuffer, m_state.atomN * 3);
}

std::vector<float *> MDSimulation::getPositionObservers(unsigned i)
{
    return {&m_state.positions[i], &m_state.positions[i+m_state.atomN], &m_state.positions[i+2*m_state.atomN]};
}

void MDSimulation::setupBondStretchKernel(float deltaTime)
{
    int ret = 0;
    deltaTime = 0.001f;

    ret = clSetKernelArg(m_bondStretchKernel, 0, sizeof(float), static_cast<void*>(&deltaTime));
    ret = clSetKernelArg(m_bondStretchKernel, 1, sizeof(cl_mem), static_cast<void*>(&m_outPositionsObj));
    ret = clSetKernelArg(m_bondStretchKernel, 2, sizeof(cl_mem), static_cast<void*>(&m_massesObj));
    ret = clSetKernelArg(m_bondStretchKernel, 3, sizeof(cl_mem), static_cast<void*>(&m_positionsObj));
    ret = clSetKernelArg(m_bondStretchKernel, 4, sizeof(cl_mem), static_cast<void*>(&m_bondsObj));
    ret = clSetKernelArg(m_bondStretchKernel, 5, sizeof(float), static_cast<void*>(&m_state.atomN));
}

void MDSimulation::setupAngleBendKernel(float deltaTime)
{
    int ret = 0;
    deltaTime = 0.0001f;


    // Set the arguments of the kernel
    ret = clSetKernelArg(m_angleBendKernel, 0, sizeof(float), static_cast<void*>(&deltaTime));
    ret = clSetKernelArg(m_angleBendKernel, 1, sizeof(cl_mem), static_cast<void*>(&m_outPositionsObj));
    ret = clSetKernelArg(m_angleBendKernel, 2, sizeof(cl_mem), static_cast<void*>(&m_positionsObj));

    ret = clSetKernelArg(m_angleBendKernel, 3, sizeof(cl_mem), static_cast<void*>(&m_anglesObj));

    ret = clSetKernelArg(m_angleBendKernel, 4, sizeof(cl_mem), static_cast<void*>(&m_massesObj));
    ret = clSetKernelArg(m_angleBendKernel, 5, sizeof(float), static_cast<void*>(&m_state.atomN));
}

void MDSimulation::setupTorsionBendKernel(float deltaTime)
{

    int ret = 0;
    deltaTime = 0.00001f;


    // Set the arguments of the kernel
    ret = clSetKernelArg(m_angleBendKernel, 0, sizeof(float), static_cast<void*>(&deltaTime));
    ret = clSetKernelArg(m_angleBendKernel, 1, sizeof(cl_mem), static_cast<void*>(&m_outPositionsObj));
    ret = clSetKernelArg(m_angleBendKernel, 2, sizeof(cl_mem), static_cast<void*>(&m_massesObj));
    ret = clSetKernelArg(m_angleBendKernel, 3, sizeof(cl_mem), static_cast<void*>(&m_positionsObj));

    ret = clSetKernelArg(m_angleBendKernel, 4, sizeof(cl_mem), static_cast<void*>(&m_torsionsObj));

    ret = clSetKernelArg(m_angleBendKernel, 5, sizeof(float), static_cast<void*>(&m_state.atomN));

}

void MDSimulation::initialiseOpenCL()
{
    // Read the kernel code from the file
    std::string kernelCode;
    std::ifstream kernelCodeStream("./computation/mdkernel.cl", std::ios::in);
    if(kernelCodeStream.is_open()){
        std::stringstream sstr;
        sstr << kernelCodeStream.rdbuf();
        kernelCode = sstr.str();
        kernelCodeStream.close();
    }else{
        throw MDSimulationError("Impossible to open " + std::string("./computation/mdkernel.cl") + ". Are you in the right directory ?");
    }

    // Get platform and device information
    cl_platform_id platform_id = nullptr;
    cl_device_id device_id = nullptr;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_DEFAULT, 1,
                          &device_id, &ret_num_devices);

    // Create an OpenCL m_context
    m_context = clCreateContext( nullptr, 1, &device_id, nullptr, nullptr, &ret);

    // Create a command queue
    m_commandQueue = clCreateCommandQueue(m_context, device_id, 0, &ret);

    // Create a m_program from the kernel source
    const char* str = kernelCode.c_str();
    const size_t sizes[1] = {kernelCode.size()};

    m_program = clCreateProgramWithSource(m_context, 1, &str, sizes, &ret);

    size_t max_working_group;
    clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_working_group, nullptr);
    this->m_max_working_group_size = static_cast<unsigned>(max_working_group);


    // Build the m_program
    ret = clBuildProgram(m_program, 1, &device_id, nullptr, nullptr, nullptr);
    if (ret != 0){
        size_t ret_val_size = 0;

        clGetProgramBuildInfo(m_program, device_id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &ret_val_size);

        char* build_log = new char[ret_val_size+1];
        build_log[ret_val_size] = 0;

        clGetProgramBuildInfo(m_program, device_id, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, nullptr);

        std::cout << build_log << std::endl;

        throw MDSimulationError("OpenCL kernel compile failed with: " + std::string(build_log));
    }
}

void MDSimulation::initialiseOpenCLMemoryObjs()
{
    int ret = 0;

    clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float), nullptr, &ret);

    m_positionsObj = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float) * 3 * m_state.atomN, nullptr, &ret);
    m_outPositionsObj =       clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float) * 3 * m_state.atomN, nullptr, &ret);
    m_chargesObj =   clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float) * m_state.atomN,     nullptr, &ret);
    m_massesObj =    clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float) * m_state.atomN,     nullptr, &ret);
    m_bondsObj =   clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float) *m_state.bondsSize,   nullptr, &ret);

    m_anglesObj =   clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float)  *m_state.angleSize,   nullptr, &ret);
    m_torsionsObj = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeof(float)  *m_state.torsionsSize,   nullptr, &ret);

    m_bondStretchKernel = clCreateKernel(m_program, "bond_stretch", &ret);
    m_angleBendKernel = clCreateKernel(m_program, "angle_bend", &ret);
    m_torsionBendKernel = clCreateKernel(m_program, "torsion_bend", &ret);
}

void MDSimulation::instCopyDataState2GPU()
{
    int ret = 0;

    ret = clEnqueueWriteBuffer(m_commandQueue, m_outPositionsObj,CL_TRUE, 0,  sizeof(float) * 3 * m_state.atomN, m_state.positions, 0, nullptr, nullptr);
    ret = clEnqueueWriteBuffer(m_commandQueue, m_positionsObj,CL_TRUE, 0,  sizeof(float) * 3 * m_state.atomN, m_state.positions, 0, nullptr, nullptr);
    ret = clEnqueueWriteBuffer(m_commandQueue, m_chargesObj,  CL_TRUE, 0,  sizeof(float) * m_state.atomN,     m_state.charges,   0, nullptr, nullptr);
    ret = clEnqueueWriteBuffer(m_commandQueue, m_bondsObj,   CL_TRUE, 0,  sizeof(float) * m_state.bondsSize,  m_state.bonds, 0, nullptr, nullptr);
    ret = clEnqueueWriteBuffer(m_commandQueue, m_massesObj,  CL_TRUE, 0,   sizeof(float) *m_state.atomN,   m_state.masses, 0, nullptr, nullptr);
    ret = clEnqueueWriteBuffer(m_commandQueue, m_anglesObj,  CL_TRUE, 0,   sizeof(float) *m_state.angleSize,m_state.angles, 0, nullptr, nullptr);
    ret = clEnqueueWriteBuffer(m_commandQueue, m_torsionsObj,  CL_TRUE, 0,   sizeof(float) *m_state.torsionsSize, m_state.torsions, 0, nullptr, nullptr);
}
