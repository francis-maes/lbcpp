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

// Container, ... -> DoubleVector
class ContainerWindowFeatureGenerator : public FeatureGenerator
{
public:
  virtual String getOutputPostFix() const
    {return T("Window");}

  virtual void getRelativeWindow(int& startPosition, size_t& windowSize) const = 0;
  virtual void getAbsoluteWindow(const Variable* inputs, int& startPosition, size_t& windowSize) const = 0;

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    TypePtr doubleVectorType = Container::getTemplateParameter(inputVariables[0]->getType());
    EnumerationPtr subFeaturesEnumeration;
    if (!DoubleVector::getTemplateParameters(context, doubleVectorType, subFeaturesEnumeration, elementsType))
      return EnumerationPtr();

    DefaultEnumerationPtr res = new DefaultEnumeration(T("WindowFeatures"));
    int startPosition;
    size_t windowSize;
    getRelativeWindow(startPosition, windowSize);
    numFeaturesPerPosition = subFeaturesEnumeration->getNumElements();
    for (int i = 0; i < (int)windowSize; ++i)
    {
      String pos = String(i + startPosition);
      res->addElementsWithPrefix(context, subFeaturesEnumeration, T("[") + pos + T("]."), pos + T("."));
    }
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();
    int startPosition;
    size_t windowSize;
    
    getAbsoluteWindow(inputs, startPosition, windowSize);

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
  size_t numFeaturesPerPosition;
};

// Container[DoubleVector[.]] -> DoubleVector[...]
class FixedContainerWindowFeatureGenerator : public ContainerWindowFeatureGenerator
{
public:
  FixedContainerWindowFeatureGenerator(size_t begin = 0, size_t size = 0)
    : begin(begin), size(size) {}

  virtual void getRelativeWindow(int& startPosition, size_t& windowSize) const
    {startPosition = (int)begin; windowSize = size;}

  virtual void getAbsoluteWindow(const Variable* inputs, int& startPosition, size_t& windowSize) const
    {startPosition = (int)begin; windowSize = size;}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(doubleVectorClass());}
 
protected:
  friend class FixedContainerWindowFeatureGeneratorClass;
  
  size_t begin;
  size_t size;
};

// Container[DoubleVector[.]], PositiveInteger -> DoubleVector[...]
class CenteredContainerWindowFeatureGenerator : public ContainerWindowFeatureGenerator
{
public:
  CenteredContainerWindowFeatureGenerator(size_t windowSize = 0)
    : windowSize(windowSize) {}

  virtual void getRelativeWindow(int& startPosition, size_t& windowSize) const
    {startPosition = -(int)(this->windowSize / 2); windowSize = this->windowSize;}

  virtual void getAbsoluteWindow(const Variable* inputs, int& startPosition, size_t& windowSize) const
    {startPosition = inputs[1].getInteger() - (int)(this->windowSize / 2); windowSize = this->windowSize;}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)containerClass(doubleVectorClass());}
  
protected:
  friend class CenteredContainerWindowFeatureGeneratorClass;

  size_t windowSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_CONTAINER_WINDOW_H_
