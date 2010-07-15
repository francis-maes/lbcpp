/*-----------------------------------------.---------------------------------.
| Filename: ResiduePerception.cpp          | Residue Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 17:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ResiduePerception.h"
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
  {
    checkInheritance(getInputType(), inputType);
    CompositePerception::addPerception(name, subPerception);
  }
}

/*
** ProteinToResiduePerception
*/
class ProteinToResiduePerception : public DecoratorPerception
{
public:
  ProteinToResiduePerception(PerceptionPtr proteinPerception = PerceptionPtr())
    : DecoratorPerception(proteinPerception) {}

  virtual TypePtr getOutputType() const
    {return decorated->getOutputType();}

  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), integerType());}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {decorated->computePerception(input[0], callback);}
};

PerceptionPtr lbcpp::proteinToResiduePerception(PerceptionPtr proteinPerception)
  {return new ProteinToResiduePerception(proteinPerception);}

void declareResiduePerceptionClasses()
{
  LBCPP_DECLARE_CLASS(ResidueCompositePerception, CompositePerception);
  LBCPP_DECLARE_CLASS(ProteinToResiduePerception, DecoratorPerception);
}
