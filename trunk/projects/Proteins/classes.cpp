/*-----------------------------------------.---------------------------------.
| Filename: classes.cpp                    | Declaration of serializable     |
| Author  : Francis Maes                   |   classes                       |
| Started : 27/03/2010 12:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "GeneratedCode/Data/Bio/Protein.lh"
using namespace lbcpp;

void declareProteinsClasses()
{
  LBCPP_DECLARE_CLASS(Protein);

  LBCPP_DECLARE_CLASS(AminoAcidSequence);
  LBCPP_DECLARE_CLASS(PositionSpecificScoringMatrix);
  LBCPP_DECLARE_CLASS(SecondaryStructureSequence);
  LBCPP_DECLARE_CLASS(SolventAccessibilitySequence);
}
