#ifndef GLOBALPROGRAMMEMANAGER_H
#define GLOBALPROGRAMMEMANAGER_H

#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <string>
#include <unordered_map>

struct ShaderProgramme{
    GLuint id;
    std::unordered_map<std::string, GLint> uniforms;
};

class GlobalProgrammeManager
{
public:
    GlobalProgrammeManager();

    std::unordered_map<std::string, ShaderProgramme> programmeAt;

    static void del(){
        if(m_instance != nullptr){
            for(const auto & pair : m_instance->programmeAt){
                glDeleteProgram(pair.second.id);
            }
            m_instance->programmeAt.clear();
            delete m_instance;
        }
    }

    static GlobalProgrammeManager* get(){
        if(m_instance == nullptr)
            m_instance = new GlobalProgrammeManager;
        return m_instance;
    }

    static GlobalProgrammeManager* m_instance;

private:
    void addProgramme(std::string path, std::string name, std::vector<std::pair<std::string, std::string>> programmeAndStoreName);
    GLuint CreateShaderProgramme(std::string vertex_file_path, std::string fragment_file_path);
};



#endif // GLOBALPROGRAMMEMANAGER_H
