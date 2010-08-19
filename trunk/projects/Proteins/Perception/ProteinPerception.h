/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.h            | Protein Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_H_
# define LBCPP_PROTEIN_PERCEPTION_H_

# include <lbcpp/Data/Perception.h>
# include "../Data/Protein.h"

namespace lbcpp
{

/*
** ProteinPerception
*/
class ProteinCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass();}
};

inline PerceptionPtr proteinLengthPerception()
  {return functionBasedPerception(proteinLengthFunction());}

/*
** ResiduePerception
*/
class ResidueCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), integerType());}

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
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

extern DecoratorPerceptionPtr proteinToResiduePairPerception(PerceptionPtr proteinPerception);
extern ResiduePairPerceptionPtr residueToResiduePairPerception(PerceptionPtr residuePerception);

extern ResiduePairPerceptionPtr separationDistanceResiduePairPerception();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
