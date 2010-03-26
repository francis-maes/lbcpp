#include <lbcpp/lbcpp.h>
#include "Data/Bio/AminoAcidSequence.lh"
#include "Data/Bio/PositionSpecificScoringMatrix.lh"
#include "Data/Bio/SecondaryStructureSequence.lh"
#include "Data/Bio/SolventAccesibilitySequence.lh"
using namespace lbcpp;

class NamedObjectMap : public Object
{
public:
  
private:
  typedef std::map<std::string, ObjectPtr> ObjectsMap;
  
  ObjectsMap objects;
};


// Data: 

// AminoAcid,
// LabelSequence: AminoAcidSequence
// VectorSequence: PositionSpecificScoringMatrix
// LabelSequence: 3 state and 8 state SecondaryStructureSequence
// LabelSequence: 2 state SolventAccesibilitySequence
// ScoreSequence: regression SolventAccesibilitySequence
// ScalarMatrix: 2 state ResidueContactMap
// ScalarMatrix: regression ResidueDistanceMap
// VectorSequence: BackboneSequence
// Object: ThirdaryStructure

int main()
{
  // LBCPP_DECLARE_CLASS
  return 0;
}
