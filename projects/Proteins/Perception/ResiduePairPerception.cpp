/*-----------------------------------------.---------------------------------.
| Filename: ResiduePairPerception.cpp      | Residue Pair Perception         |
| Author  : Francis Maes                   |                                 |
| Started : 15/07/2010 17:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ResiduePairPerception.h"
using namespace lbcpp;

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
  {
    checkInheritance(getInputType(), inputType);
    CompositePerception::addPerception(name, subPerception);
  }
}

/*
** ProteinToResiduePairPerception
*/
class ProteinToResiduePairPerception : public DecoratorPerception
{
public:
  ProteinToResiduePairPerception(PerceptionPtr proteinPerception = PerceptionPtr())
    : DecoratorPerception(proteinPerception) {}

  virtual TypePtr getOutputType() const
    {return decorated->getOutputType();}

  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {decorated->computePerception(input[0], callback);}
};

PerceptionPtr lbcpp::proteinToResiduePairPerception(PerceptionPtr proteinPerception)
  {return new ProteinToResiduePairPerception(proteinPerception);}

/*
** ResidueToResiduePairPerception
*/
class ResidueToResiduePairPerception : public Perception
{
public:
  ResidueToResiduePairPerception(PerceptionPtr residuePerception = PerceptionPtr())
    : residuePerception(residuePerception) {}

  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}

  virtual size_t getNumOutputVariables() const
    {return 2;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return residuePerception->getOutputType();}

  virtual String getOutputVariableName(size_t index) const
    {return index ? T("residue2") : T("residue1");}

  virtual PerceptionPtr getOutputVariableGenerator(size_t index) const
    {return residuePerception;}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    Variable protein = input[0];
    Variable positionPair = input[1];
    callback->sense(0, residuePerception, Variable::pair(protein, positionPair[0]));
    callback->sense(1, residuePerception, Variable::pair(protein, positionPair[1]));
  }

protected:
  PerceptionPtr residuePerception;
};

PerceptionPtr lbcpp::residueToResiduePairPerception(PerceptionPtr residuePerception)
  {return new ResidueToResiduePairPerception(residuePerception);}

void declareResiduePairPerceptionClasses()
{
  LBCPP_DECLARE_CLASS(ProteinToResiduePairPerception, DecoratorPerception);
  LBCPP_DECLARE_CLASS(ResidueToResiduePairPerception, Perception);

  LBCPP_DECLARE_CLASS(ResiduePairCompositePerception, CompositePerception);
}
