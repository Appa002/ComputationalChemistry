#ifndef DATA_ELEMENTS_H
#define DATA_ELEMENTS_H

#include <unordered_map>
#include <string>

struct DATA_Element{
    DATA_Element() = default;
    DATA_Element(std::string s) : symbol(s) {}
    DATA_Element(std::string s, std::string ft, float m, float br, float va, float nbd, float pc)
        : symbol(s), formatedType(ft), mass(m), bondRadius(br), valenzAngle(va), noneBondDistance(nbd), partialCharge(pc) {}
    std::string symbol;
    std::string formatedType;
    float mass = 0;

    float bondRadius = 0;
    float valenzAngle = 0;

    float noneBondDistance = 0;
    float partialCharge = 0;
};

class DATA_Elements
{
public:
    DATA_Elements() = default;

    const std::unordered_map<std::string,
    std::unordered_map<std::string,
    std::unordered_map<std::string, DATA_Element>>> data {

#include <elementdata.txt>
        // This works because #include basically copy-paste s the content from the file which is to be inculded to where the include directive is.
        // The data in elementdata.txt is formated such that it sets up DATA_Elements::data correctly using intilizer lists.
    };
};

#endif // DATA_ELEMENTS_H
