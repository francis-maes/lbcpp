/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.h            | Protein Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_H_
# define LBCPP_PROTEIN_PERCEPTION_H_

# include <lbcpp/Function/Perception.h>
# include "../Data/Protein.h"

namespace lbcpp
{

/*
** ProteinFunction
*/
extern FunctionPtr residueToSelectPairSequencesFunction(int index1, int index2);

/*
** ProteinPerception
*/
class ProteinCompositePerception : public CompositePerception
{
public:
  ProteinCompositePerception() : CompositePerception(proteinClass(), T("protein")) {}
};

class ResidueCompositePerception : public CompositePerception
{
public:
  ResidueCompositePerception()
    : CompositePerception(pairClass(proteinClass(), positiveIntegerType()), T("residue")) {}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

class ResiduePairCompositePerception : public CompositePerception
{
public:
  ResiduePairCompositePerception()
    : CompositePerception(pairClass(proteinClass(), pairClass(positiveIntegerType(), positiveIntegerType())), T("residue pair")) {}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

// Protein Perception
inline PerceptionPtr proteinLengthPerception()
  {return functionBasedPerception(proteinLengthFunction());}

// Residue Perception
extern DecoratorPerceptionPtr proteinToResiduePerception(PerceptionPtr proteinPerception);

// Residue Pair Perception
extern DecoratorPerceptionPtr proteinToResiduePairPerception(PerceptionPtr proteinPerception);
extern PerceptionPtr residueToResiduePairPerception(PerceptionPtr residuePerception);
extern PerceptionPtr separationDistanceResiduePairPerception();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
