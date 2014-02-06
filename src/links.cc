#include "links.h"

using namespace metacog;

string mclLink::dotDescription() {
  return sourceNode()->dotLabel() + " -> " + 
    destinationNode()->dotLabel() + ";";  
}

string interontologicalLink::dotDescription() {
  return sourceNode()->dotLabel() + " -> " + 
    destinationNode()->dotLabel() + " [style = dotted];";  
}
