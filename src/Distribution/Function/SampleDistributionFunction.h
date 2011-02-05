/*-----------------------------------------.---------------------------------.
| Filename: SampleDistributionFunction.h   | Sample Distribution Function    |
| Author  : Francis Maes                   |                                 |
| Started : 31/01/2011 22:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DISTRIBUTION_FUNCTION_SAMPLE_H_
# define LBCPP_DISTRIBUTION_FUNCTION_SAMPLE_H_

# include <lbcpp/Function/Function.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

  // container[distribution[T]] => container[T]
class SampleDistributionContainerFunction : public Function
{
public:
  SampleDistributionContainerFunction(bool sampleBest = true)
    : sampleBest(sampleBest) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return containerClass(distributionClass(anyType));}

  virtual String getOutputPostFix() const
    {return T("Discretized");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr distributionType = Container::getTemplateParameter(inputVariables[0]->getType());
    TypePtr elementsType = Distribution::getTemplateParameter(distributionType);
    return vectorClass(elementsType);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& container = inputs[0].getObjectAndCast<Container>();

    RandomGeneratorPtr random = RandomGenerator::getInstance();
    size_t n = container->getNumElements();
    VectorPtr res = vector(elementsType, n);
    for (size_t i = 0; i < n; ++i)
    {
      DistributionPtr distribution = container->getElement(i).getObjectAndCast<Distribution>();
      if (distribution)
        res->setElement(i, sampleBest ? distribution->sampleBest(random) : distribution->sample(random));
    }
    return res;
  };

protected:
  friend class SampleDistributionContainerFunctionClass;

  TypePtr elementsType;
  bool sampleBest;
};

class SampleDistributionFunction : public ProxyFunction
{
public:
  SampleDistributionFunction(bool sampleBest = true)
    : sampleBest(sampleBest) {}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    if (inputVariables.size() == 1 && inputVariables[0]->getType()->inheritsFrom(containerClass(distributionClass(anyType))))
      return new SampleDistributionContainerFunction(sampleBest);
    return FunctionPtr();
  }

protected:
  friend class SampleDistributionFunctionClass;
  bool sampleBest;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DISTRIBUTION_FUNCTION_SAMPLE_H_
