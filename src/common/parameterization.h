#ifndef PARAMETERIZATION_H
#define PARAMETERIZATION_H

#include "atom.h"

class Parameterization
{
public:
    Parameterization();

    static float r (Atom* i, Atom* j);
    static float kBond (Atom* i, Atom* j);
    static float bondOrder(Atom* i, Atom* j);
    static float kValenz(Atom* i, Atom* j, Atom* k);
    static std::vector<float> fourierValenz(Atom* i, Atom* j, Atom* k);
    static float torsionalFirstExpansion(Atom* central);

};

#endif // PARAMETERIZATION_H
