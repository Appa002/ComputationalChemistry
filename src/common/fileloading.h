#ifndef FILELOADING_H
#define FILELOADING_H

#include <rapidxml/rapidxml.hpp>
#include <common/atom.h>

class FileLoading
{
public:
    FileLoading();

    static Atom* loadMolecule(std::string path);
    static std::string storeMolecule(std::string path, std::string name, Atom* molecule);

    static std::string getDayTime();


};

#endif // FILELOADING_H
