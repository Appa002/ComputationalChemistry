#include "fileloading.h"
#include <rapidxml/rapidxml_print.hpp>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <functional>

FileLoading::FileLoading()
{

}

Atom *FileLoading::loadMolecule(std::string path)
{
    using namespace rapidxml;
    std::ifstream file;
    file.open(path, std::ios::in);
    if(!file.is_open())
        return nullptr;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();

    if(!file.good())
        return nullptr;

    file.close();

    xml_document<> doc;
    char* dataRaw = doc.allocate_string(data.c_str());

    try {
        doc.parse<0>(dataRaw);
    } catch (rapidxml::parse_error&) {
        return nullptr;
    }

    xml_node<>* base = doc.first_node("molecule");
    if(base == nullptr)
        return nullptr;

    std::function<Atom*(xml_node<>*, Atom*)> dfs = [&](xml_node<>* node, Atom* parent) -> Atom*{
#define THROW_FIRST_NODE(SYM) node->first_node( SYM ) ? node->first_node( SYM )->value() : throw std::runtime_error("XML misshapen")

        Atom* atom = new Atom;
        atom->parent = parent;

        try {
            atom->name = THROW_FIRST_NODE("name");
            atom->symbol = THROW_FIRST_NODE("symbol");

            atom->x = std::stof(THROW_FIRST_NODE("x"));
            atom->y = std::stof(THROW_FIRST_NODE("y"));
            atom->z = std::stof(THROW_FIRST_NODE("z"));

            atom->mass = std::stof(THROW_FIRST_NODE("mass"));
            atom->bondRadius = std::stof(THROW_FIRST_NODE("bond_radius"));
            atom->noneBondDistance = std::stof(THROW_FIRST_NODE("nonebond_distance"));

            atom->partialCharge =  std::stof(THROW_FIRST_NODE("partial_charge"));

            atom->display_chargeParam = THROW_FIRST_NODE("display_charge");
            atom->display_type = THROW_FIRST_NODE("display_type");

            atom->valenzAngle = std::stof(THROW_FIRST_NODE("valenz_angle"));

            atom->torsionalAngle = std::stof(THROW_FIRST_NODE("torsional_angle"));
            atom->torsionalV = std::stof(THROW_FIRST_NODE("torsional_V"));
            atom->torsionalN = std::stof(THROW_FIRST_NODE("torsional_N"));

            xml_node<>* bondOrders = node->first_node( "bond_orders" ) ? node->first_node( "bond_orders" ) : throw std::runtime_error("XML misshapen");
            for(xml_node<>* bondOrder = bondOrders->first_node(); bondOrder; bondOrder = bondOrder->next_sibling()){
                atom->bondOrders.push_back(std::stof(bondOrder->value()));
            }

        } catch (std::runtime_error&) {
            delete atom;
            return nullptr;
        } catch (std::invalid_argument&){
            delete atom;
            return nullptr;
        } catch (std::out_of_range&){
            delete atom;
            return nullptr;
        }


        for(xml_node<>* child = node->first_node("children")->first_node(); child; child = child->next_sibling()){
            Atom* next = dfs(child, atom);
            if(next == nullptr){
                delete atom;
                return nullptr;
            }
            atom->children.push_back(next);
        }

        return atom;
    };

    Atom* out = dfs(base->first_node(), nullptr);
    if(out){
        xml_attribute<>* attrib = base->first_attribute("name");
        if(!attrib){
            delete out;
            return nullptr;
        }

        out->display_moleculeName = attrib->value();
    }

    return out;
}

std::string FileLoading::storeMolecule(std::string path, std::string name, Atom *molecule)
{
    using namespace rapidxml;

    xml_document<> doc;
    xml_node<>* base = doc.allocate_node(node_element, "molecule");

    base->append_attribute(doc.allocate_attribute("name", name.c_str()));
    base->append_attribute(doc.allocate_attribute("date", doc.allocate_string(getDayTime().c_str()))); // rapidxml requires the string to remain accesible for the life-time of `doc`

    doc.append_node(base);


    std::function<void(xml_node<>*, Atom*)> dfs = [&](xml_node<>* node, Atom* atom) -> void{
        #define F(sym) doc.allocate_string(std::to_string(atom->sym).c_str()) // for float
        #define S(sym) doc.allocate_string(atom->sym.c_str()) // for string

        xml_node<>* next = doc.allocate_node(node_element, "atom");

        next->append_node(doc.allocate_node(node_element, "name", S(name)));
        next->append_node(doc.allocate_node(node_element, "symbol", S(symbol)));

        next->append_node(doc.allocate_node(node_element, "x", F(x)));
        next->append_node(doc.allocate_node(node_element, "y", F(y)));
        next->append_node(doc.allocate_node(node_element, "z", F(z)));

        next->append_node(doc.allocate_node(node_element, "mass", F(mass)));
        next->append_node(doc.allocate_node(node_element, "bond_radius", F(bondRadius)));
        next->append_node(doc.allocate_node(node_element, "nonebond_distance", F(noneBondDistance)));

        next->append_node(doc.allocate_node(node_element, "partial_charge", F(partialCharge)));

        next->append_node(doc.allocate_node(node_element, "display_charge", S(display_chargeParam)));
        next->append_node(doc.allocate_node(node_element, "display_type", S(display_type)));

        next->append_node(doc.allocate_node(node_element, "valenz_angle", F(valenzAngle)));

        next->append_node(doc.allocate_node(node_element, "torsional_angle", F(torsionalAngle)));
        next->append_node(doc.allocate_node(node_element, "torsional_V", F(torsionalV)));
        next->append_node(doc.allocate_node(node_element, "torsional_N", F(torsionalN)));

        xml_node<>* bondOrder = doc.allocate_node(node_element, "bond_orders");
        for(float order : atom->bondOrders){
            bondOrder->append_node(doc.allocate_node(node_element, "bond_order", doc.allocate_string(std::to_string(order).c_str())));
        }

        next->append_node(bondOrder);

        node->append_node(next);

        xml_node<>* nextChild = doc.allocate_node(node_element, "children");
        next->append_node(nextChild);

        for(auto const & child : atom->children)
            dfs(nextChild, child);


    };

    dfs(base, molecule);

    std::ofstream file;
    file.open(path, std::ios::trunc | std::ios::out);
    if(!file.is_open())
        return "Could not open/create file " + path;

    file << doc << std::endl;

    if(!file.good())
        return "A file error occured " + path;

    file.close();


    return std::string();
}

std::string FileLoading::getDayTime()
{
    // see: https://stackoverflow.com/questions/16357999/current-date-and-time-as-string
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t); // may cause data races, but that's fine here

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");

    return oss.str();;
}
