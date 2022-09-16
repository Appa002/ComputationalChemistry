#ifndef ATOM_H
#define ATOM_H

#include <string>
#include <vector>
#include <unordered_map>

class Atom
{
public:
    Atom();
  //  Atom(float x, float y, float z, float charge, float mass, std::string name, std::string symbol, int idx)

    float x = 0;
    float y = 0;
    float z = 0;

    float mass = 0;

    std::vector<Atom*> children;
    Atom* parent = nullptr;

    std::string name;
    std::string symbol;

    unsigned idx = 0;

    float bondRadius = 0;
    float noneBondDistance = 0;
    float partialCharge = 0;
    float valenzAngle = 0;
    float torsionalAngle = 0;
    float torsionalN = 0;
    float torsionalV = 0;
    std::vector<float> bondOrders = {};

    std::string display_type = "Linear";
    float display_inversion = 1.0f;
    std::string display_chargeParam = "0";
    std::string display_moleculeName;
    std::unordered_map<std::string, std::vector<float>> display_colourMap;


    size_t countSubTree();
    void allChains(std::vector<std::vector<Atom*>>& out, std::vector<Atom*> cur = {});
};

#endif // ATOM_H
