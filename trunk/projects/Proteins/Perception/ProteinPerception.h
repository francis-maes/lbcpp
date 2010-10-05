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
extern FunctionPtr proteinToVariableFunction(int variableNumber);
extern FunctionPtr residueToSelectPairSequencesFunction(int index1, int index2);

/*
** ProteinPerception
*/
class ProteinCompositePerception : public CompositePerception
{
public:
  ProteinCompositePerception() : CompositePerception(proteinClass(), T("protein")) {}
};

inline PerceptionPtr proteinLengthPerception()
  {return functionBasedPerception(proteinLengthFunction());}

/*
** ResiduePerception
*/
class ResiduePerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), integerType());}
};

typedef ReferenceCountedObjectPtr<ResiduePerception> ResiduePerceptionPtr;

extern ResiduePerceptionPtr positionResiduePerception();
extern ResiduePerceptionPtr indexResiduePerception();
extern ResiduePerceptionPtr boundsProximityResiduePerception(size_t outOfBoundWindowSize);
  
class ResidueCompositePerception : public CompositePerception
{
public:
  ResidueCompositePerception()
    : CompositePerception(pairType(proteinClass(), integerType()), T("residue")) {}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

extern DecoratorPerceptionPtr proteinToResiduePerception(PerceptionPtr proteinPerception);

/*
** ResiduePairPerception
*/
class ResiduePairPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}
};

typedef ReferenceCountedObjectPtr<ResiduePairPerception> ResiduePairPerceptionPtr;

class ResiduePairCompositePerception : public CompositePerception
{
public:
  ResiduePairCompositePerception()
    : CompositePerception(pairType(proteinClass(), pairType(integerType(), integerType())), T("residue pair")) {}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

extern DecoratorPerceptionPtr proteinToResiduePairPerception(PerceptionPtr proteinPerception);
extern ResiduePairPerceptionPtr residueToResiduePairPerception(PerceptionPtr residuePerception);

extern ResiduePairPerceptionPtr separationDistanceResiduePairPerception();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
