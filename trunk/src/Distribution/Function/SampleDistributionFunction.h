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

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    TypePtr distributionType;
    if (!Container::getTemplateParameter(context, inputVariable->getType(), distributionType) || !Distribution::getTemplateParameter(context, distributionType, elementsType))
      return VariableSignaturePtr();
    if (!elementsType)
      return VariableSignaturePtr();
    return new VariableSignature(vectorClass(elementsType), inputVariable->getName() + T("Discretized"), inputVariable->getShortName() + T("d"));
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
