#include "parameterization.h"

#include <cmath>

Parameterization::Parameterization()
{

}

float Parameterization::r(Atom *i, Atom *j)
{
    float rbo;
    if(bondOrder(i, j) == 0)
        rbo = 0.0f;
    else
        rbo = (-0.1332f)*(i->bondRadius + j->bondRadius) * static_cast<float>(log(bondOrder(i, j)));
    float ren = (i->bondRadius * j->bondRadius * pow( sqrt(i->noneBondDistance) - sqrt(j->noneBondDistance), 2)) /
            (i->noneBondDistance * i->bondRadius + j->noneBondDistance * j->bondRadius);

    return i->bondRadius + j->bondRadius + rbo + ren;
}

float Parameterization::kBond(Atom *i, Atom *j)
{
    return (644.12f * i->partialCharge * j->partialCharge) / pow(r(i, j), 3);
}

float Parameterization::bondOrder(Atom *i, Atom *j)
{
    float n = 0;
    unsigned idx = 0;
    for(auto const& child : i->children){
        if (child == j){
           n = i->bondOrders[idx];
        }
        ++idx;
    }
    return n;
}

float Parameterization::kValenz(Atom *i, Atom *j, Atom *k)
{
    float x = r(i, k);
    float beta = 664.12f/(r(i, j) * r(j, k));
    float first = (beta * i->partialCharge * k->partialCharge * r(i, j) * r(j, k)) / pow(r(i, k), 5);

    float lhs = pow(r(i, j), 3) * r(j, k) * (1 - pow(cos(j->valenzAngle), 2));
    float rhs = pow(r(i, k), 2) * cos(j->valenzAngle);
    return first * (lhs - rhs);
}

std::vector<float> Parameterization::fourierValenz(Atom *, Atom *j, Atom *)
{
    float c2 = 1/(4*pow(sin(j->valenzAngle), 2));
    float c1 = -4*c2*cos(j->valenzAngle);
    float c0 = c2*(2*pow(cos(j->valenzAngle), 2) + 1);
    return {c0, c1, c2};
}

float Parameterization::torsionalFirstExpansion(Atom *central)
{
    return cos(central->torsionalAngle * central->torsionalN);
}






