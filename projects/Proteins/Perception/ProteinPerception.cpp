/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.cpp          | Protein Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinPerception.h"
using namespace lbcpp;

/*
** ResidueCompositePerception
*/
void ResidueCompositePerception::addPerception(const String& name, PerceptionPtr subPerception)
{
  TypePtr inputType = subPerception->getInputType();
  if (inputType == proteinClass())
    CompositePerception::addPerception(name, proteinToResiduePerception(subPerception));
  else
    CompositePerception::addPerception(name, subPerception);
}

/*
** ResiduePairCompositePerception
*/
void ResiduePairCompositePerception::addPerception(const String& name, PerceptionPtr subPerception)
{
  TypePtr inputType = subPerception->getInputType();
  if (inputType == proteinClass())
    CompositePerception::addPerception(name, proteinToResiduePairPerception(subPerception));
  else if (inputType == pairType(proteinClass(), integerType()))
    CompositePerception::addPerception(name, residueToResiduePairPerception(subPerception));
  else
    CompositePerception::addPerception(name, subPerception);
}
