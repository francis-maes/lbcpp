/*-----------------------------------------.---------------------------------.
| Filename: ObjectDoubleMembersFeatureG...h| Create one feature per object   |
| Author  : Francis Maes                   |  double member variable         |
| Started : 12/04/2011 11:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_ATOMIC_OBJECT_DOUBLE_MEMBERS_FEATURE_GENERATOR_H_
# define LBCPP_FEATURE_GENERATOR_ATOMIC_OBJECT_DOUBLE_MEMBERS_FEATURE_GENERATOR_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

// Object -> DoubleVector
class ObjectDoubleMembersFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return objectClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    ClassPtr inputClass = inputVariables[0]->getType();
    
    DefaultEnumerationPtr res = new DefaultEnumeration(inputClass->getName() + T(" double values"));
    size_t n = inputClass->getNumMemberVariables();
    variableIndices.reserve(n);
    res->reserveElements(n);
    for (size_t i = 0; i < n; ++i)
    {
      VariableSignaturePtr stateVariable = inputClass->getMemberVariable(i);
      if (stateVariable->getType()->inheritsFrom(doubleType))
      {
        res->addElement(context, stateVariable->getName(), String::empty, stateVariable->getShortName(), stateVariable->getDescription());
        variableIndices.push_back(i);
      }
    }

    if (res->getNumElements() == 0)
    {
      context.errorCallback(T("No double member variables in class ") + inputClass->getName());
      return EnumerationPtr();
    }
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ObjectPtr& object = inputs[0].getObject();
    if (object)
      for (size_t i = 0; i < variableIndices.size(); ++i)
      {
        Variable v = object->getVariable(variableIndices[i]);
        if (v.exists())
          callback.sense(i, v.getDouble());
      }
  }

protected:
  std::vector<size_t> variableIndices;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_ATOMIC_OBJECT_DOUBLE_MEMBERS_FEATURE_GENERATOR_H_
