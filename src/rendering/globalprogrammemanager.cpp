#include "globalprogrammemanager.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>

GlobalProgrammeManager* GlobalProgrammeManager::m_instance = nullptr;

GlobalProgrammeManager::GlobalProgrammeManager()
{
    addProgramme("./shaders/SphereShader", "sphere", {{"MVP", "mvp"}, {"atom_colour", "atomColour"}});
    addProgramme("./shaders/LineShader", "line", {{"mvp", "mvp"}});
    addProgramme("./shaders/GridShader", "grid", {{"mvp", "mvp"}});


}

void GlobalProgrammeManager::addProgramme(std::string path, std::string name, std::vector<std::pair<std::string, std::string> > programmeAndStoreName)
{
    GLuint programme = CreateShaderProgramme(path + ".vert", path + ".frag");
    this->programmeAt[name] = {programme, {}};
    auto& ref = this->programmeAt[name];

    for (auto const & pair : programmeAndStoreName){
        GLint loc = glGetUniformLocation(programme, pair.first.c_str());
        ref.uniforms[pair.second] = loc;
    }
}

GLuint GlobalProgrammeManager::CreateShaderProgramme(std::string vertex_file_path, std::string fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }else{
        throw std::runtime_error("Impossible to open " + vertex_file_path + ". Are you in the right directory ?");
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }else{
        throw std::runtime_error("Impossible to open " + fragment_file_path + ". Are you in the right directory ?");
        getchar();
        return 0;
    }


    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        throw std::runtime_error(&VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        std::cout << (char*)(&FragmentShaderErrorMessage[0]) << std::endl;
        throw std::runtime_error(&FragmentShaderErrorMessage[0]);
    }

    // Link the program
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        throw std::runtime_error(&ProgramErrorMessage[0]);
    }

    //glDetachShader(ProgramID, VertexShaderID);
   // glDetachShader(ProgramID, FragmentShaderID);

    //glDeleteShader(VertexShaderID);
    //glDeleteShader(FragmentShaderID);

    return ProgramID;
}
