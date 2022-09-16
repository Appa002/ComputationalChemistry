#include "atom.h"

Atom::Atom()
{

}

size_t Atom::countSubTree()
{
    size_t out = 1;
    for(auto const & child : children){
        out += child->countSubTree();
    }
    return out;
}

void Atom::allChains(std::vector<std::vector<Atom*>>& out,  std::vector<Atom*> cur)
{

   cur.push_back(this);

   if(children.empty()){
       out.push_back(cur);
       return;
   }


   for(auto itr = children.begin(); itr != children.end(); ++itr){
       if(itr == children.begin()){
           (*itr)->allChains(out, cur);
       } else{
           auto copy = cur;
           (*itr)->allChains(out, copy);
       }

   }

}
