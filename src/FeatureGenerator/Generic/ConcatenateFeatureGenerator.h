/*-----------------------------------------.---------------------------------.
| Filename: ConcatenateFeatureGenerator.h  | Concatenate Feature Generator   |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_GENERIC_CONCATENATE_H_
# define LBCPP_FEATURE_GENERATOR_GENERIC_CONCATENATE_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class ConcatenateFeatureGenerator : public FeatureGenerator
{
public:
  ConcatenateFeatureGenerator(bool lazy = true)
    : FeatureGenerator(lazy) {}

  virtual ClassPtr getLazyOutputType(EnumerationPtr featuresEnumeration, TypePtr featuresType) const
    {return compositeDoubleVectorClass(featuresEnumeration, featuresType);}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    size_t numInputs = getNumInputs();

    if (!numInputs)
    {
      context.errorCallback(T("No inputs"));
      return EnumerationPtr();
    }
    for (size_t i = 0; i < numInputs; ++i)
      if (!checkInputType(context, i, doubleVectorClass(enumValueType)))
        return EnumerationPtr();

    DefaultEnumerationPtr elementsEnumeration = new DefaultEnumeration(T("ConcatenatedFeatures"));
    elementsType = TypePtr();
    shifts.resize(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
    {
      const VariableSignaturePtr& inputVariable = getInputVariable(i);

      shifts[i] = elementsEnumeration->getNumElements();
      EnumerationPtr subElementsEnumeration;
      TypePtr subElementsType;
      if (!DoubleVector::getTemplateParameters(context, inputVariable->getType(), subElementsEnumeration, subElementsType))
        return EnumerationPtr();
      elementsEnumeration->addElementsWithPrefix(context, subElementsEnumeration, inputVariable->getName() + T("."), inputVariable->getShortName() + T("."));
      if (i == 0)
        elementsType = subElementsType;
      else
        elementsType = Type::findCommonBaseType(elementsType, subElementsType);
    }
    if (!elementsType->inheritsFrom(doubleType))
    {
      context.errorCallback(T("All elements do not inherit from double"));
      return EnumerationPtr();
    }

    outputName = T("Concatenate");
    outputShortName = T("Concat");
    return elementsEnumeration;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    for (size_t i = 0; i < shifts.size(); ++i)
    {
      const DoubleVectorPtr& subVector = inputs[i].getObjectAndCast<DoubleVector>();
      callback.sense(shifts[i], subVector, 1.0);
    }
  }

  virtual DoubleVectorPtr toLazyVector(const Variable* inputs) const
  {
    CompositeDoubleVectorPtr res = new CompositeDoubleVector(getOutputType());
    for (size_t i = 0; i < shifts.size(); ++i)
      res->appendSubVector(shifts[i], inputs[i].getObjectAndCast<DoubleVector>());
    return res;
  }

  virtual DoubleVectorPtr toComputedVector(const Variable* inputs) const
  {
    SparseDoubleVectorPtr res = new SparseDoubleVector(getOutputType());
    for (size_t i = 0; i < shifts.size(); ++i)
      inputs[i].getObjectAndCast<DoubleVector>()->appendTo(res, shifts[i]);
    return res;
  }

private:
  friend class ConcatenateFeatureGeneratorClass;

  bool lazy;
  std::vector<size_t> shifts;
  TypePtr elementsType;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_CONCATENATE_H_
