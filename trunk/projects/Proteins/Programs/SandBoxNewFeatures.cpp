/*-----------------------------------------.---------------------------------.
| Filename: SandBoxNewFeatures.cpp         | New Feature Generators          |
| Author  : Francis Maes                   |                                 |
| Started : 20/08/2010 19:07               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Data/Protein.h" 
#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Inference/ContactMapInference.h"
#include "Perception/PerceptionToFeatures.h"
#include "Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

extern void declareProteinClasses();
/*
class EnumValueToFeaturesPerception : public Perception
{
public:
  EnumValueToFeaturesPerception(EnumerationPtr enumeration)
    : enumeration(enumeration) {}

  virtual TypePtr getInputType() const
    {return enumeration;}

  virtual size_t getNumOutputVariables() const
    {return enumeration->getNumElements() + 1;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return doubleType();}

  virtual String getOutputVariableName(size_t index) const
    {return Variable(index, enumeration).toString();}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.getType()->inheritsFrom(enumeration));
    callback->sense((size_t)input.getInteger(), 1.0);
  }
  
private:
  EnumerationPtr enumeration;
};

class PerceptionToFeatures : public ModifierPerception
{
public:
  PerceptionToFeatures(PerceptionPtr decorated)
    : ModifierPerception(decorated) {}

  virtual PerceptionPtr getModifiedPerception(size_t index, TypePtr valueType) const
  {
    if (valueType->inheritsFrom(doubleType()))
      return PerceptionPtr();
    if (valueType->inheritsFrom(enumValueType()))
      return PerceptionPtr(new EnumValueToFeaturesPerception(valueType));

    // to be continued
    return PerceptionPtr();
  }

  virtual ObjectPtr clone() const
    {return new PerceptionToFeatures(decorated);}
};
*/

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  Variable protein = Variable::createFromFile(workingDirectory.getChildFile(T("PDB30Medium/1A3A.xml")));
  if (!protein)
    return 1;

  
  ProteinInferenceFactory factory;
  PerceptionPtr perception = factory.createResiduePerception(String::empty);

  perception = perceptionToFeatures(perception);
  Variable input = Variable::pair(protein, 3);
  Variable output = perception->compute(input);
  output.printRecursively(std::cout);
  return 0;
}
