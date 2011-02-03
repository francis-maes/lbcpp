/*-----------------------------------------.---------------------------------.
| Filename: WindowFeatureGenerator.h       | Window Feature Generator        |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2011 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPERATOR_FEATURE_GENERATOR_WINDOW_H_
# define LBCPP_OPERATOR_FEATURE_GENERATOR_WINDOW_H_

# include <lbcpp/Operator/FeatureGenerator.h>

namespace lbcpp
{

// Container[DoubleVector[.]], PositiveInteger -> DoubleVector[Window[.]]
class WindowFeatureGenerator : public FeatureGenerator
{
public:
  WindowFeatureGenerator(size_t windowSize = 0)
    : windowSize(windowSize) {}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    TypePtr containerElementsType;
    EnumerationPtr subFeaturesEnumeration;

    if (!checkNumInputs(context, 2) ||
        !checkInputType(context, 0, containerClass(doubleVectorClass())) ||
        !checkInputType(context, 1, positiveIntegerType) ||
        !Container::getTemplateParameter(context, getInputType(0), containerElementsType) ||
        !DoubleVector::getTemplateParameters(context, containerElementsType, subFeaturesEnumeration, elementsType))
      return EnumerationPtr();

    DefaultEnumerationPtr res = new DefaultEnumeration(T("WindowFeatures"));
    int startPosition = - (int)(windowSize / 2);
    numFeaturesPerPosition = subFeaturesEnumeration->getNumElements();
    for (size_t i = 0; i < windowSize; ++i)
    {
      String pos = String((int)i + startPosition);
      res->addElementsWithPrefix(context, subFeaturesEnumeration, T("[") + pos + T("]."), pos + T("."));
    }
    outputName = T("Window");
    outputShortName = T("Win");
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();
    int position = inputs[1].getInteger();
    
    int startPosition = position - (int)(windowSize / 2);
    
    int n = (int)container->getNumElements();
    for (size_t i = 0; i < windowSize; ++i)
    {
      int position = startPosition + (int)i;
      if (position >= 0 && position < n)
      {
        DoubleVectorPtr variable = container->getElement(position).getObjectAndCast<DoubleVector>();
        callback.sense(i * numFeaturesPerPosition, variable, 1.0);
      }
    }
  }

protected:
  friend class WindowFeatureGeneratorClass;

  size_t windowSize;
  size_t numFeaturesPerPosition;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPERATOR_FEATURE_GENERATOR_WINDOW_H_
