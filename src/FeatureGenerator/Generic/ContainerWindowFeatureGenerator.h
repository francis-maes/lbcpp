/*-----------------------------------------.---------------------------------.
| Filename: ContainerWindowFeatureGenerator.h| Container Window Features     |
| Author  : Francis Maes                   |                                 |
| Started : 03/02/2011 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_GENERIC_CONTAINER_WINDOW_H_
# define LBCPP_FEATURE_GENERATOR_GENERIC_CONTAINER_WINDOW_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

// Container[DoubleVector[.]], PositiveInteger -> DoubleVector[.]
class ContainerWindowFeatureGenerator : public FeatureGenerator
{
public:
  ContainerWindowFeatureGenerator(size_t windowSize = 0)
    : windowSize(windowSize) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)containerClass(doubleVectorClass());}
  
  virtual String getOutputPostFix() const
    {return T("Window");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    TypePtr containerElementsType = Container::getTemplateParameter(inputVariables[0]->getType());
    EnumerationPtr subFeaturesEnumeration;
    if (!DoubleVector::getTemplateParameters(context, containerElementsType, subFeaturesEnumeration, elementsType))
      return EnumerationPtr();

    DefaultEnumerationPtr res = new DefaultEnumeration(T("WindowFeatures"));
    int startPosition = - (int)(windowSize / 2);
    numFeaturesPerPosition = subFeaturesEnumeration->getNumElements();
    for (size_t i = 0; i < windowSize; ++i)
    {
      String pos = String((int)i + startPosition);
      res->addElementsWithPrefix(context, subFeaturesEnumeration, T("[") + pos + T("]."), pos + T("."));
    }
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();
    int position = inputs[1].getInteger();
    int startPosition = position - (int)(windowSize / 2);
    int n = (int)container->getNumElements();

    ObjectVectorPtr objectVector = container.dynamicCast<ObjectVector>();
    if (objectVector)
    {
      // fast version
      const std::vector<DoubleVectorPtr>& doubleVectors = objectVector->getObjectsAndCast<DoubleVector>();
      for (size_t i = 0; i < windowSize; ++i)
      {
        int position = startPosition + (int)i;
        if (position >= 0 && position < n)
        {
          if (doubleVectors[position])
            callback.sense(i * numFeaturesPerPosition, doubleVectors[position], 1.0);
        }
      }
    }
    else
    {
      // generic version
      for (size_t i = 0; i < windowSize; ++i)
      {
        int position = startPosition + (int)i;
        if (position >= 0 && position < n)
        {
          DoubleVectorPtr variable = container->getElement(position).getObjectAndCast<DoubleVector>();
          if (variable)
            callback.sense(i * numFeaturesPerPosition, variable, 1.0);
        }
      }
    }
  }

protected:
  friend class ContainerWindowFeatureGeneratorClass;

  size_t windowSize;
  size_t numFeaturesPerPosition;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_CONTAINER_WINDOW_H_
