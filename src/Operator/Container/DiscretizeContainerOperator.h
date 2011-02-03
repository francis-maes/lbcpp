/*-----------------------------------------.---------------------------------.
| Filename: DiscretizeContainerOperator.h  | Discretize Container Function   |
| Author  : Francis Maes                   |                                 |
| Started : 31/01/2011 22:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OPERATOR_DISCRETIZE_CONTAINER_H_
# define LBCPP_FUNCTION_OPERATOR_DISCRETIZE_CONTAINER_H_

# include <lbcpp/Operator/Operator.h>
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

  // container[distribution[T]] => container[T]
class DiscretizeContainerOperator : public Function
{
public:
  DiscretizeContainerOperator(bool sampleBest = true)
    : sampleBest(sampleBest) {}

  virtual VariableSignaturePtr initializeFunction(ExecutionContext& context)
  {
    if (!checkNumInputs(context, 1))
      return VariableSignaturePtr();
    VariableSignaturePtr inputVariable = getInputVariable(0);
    TypePtr distributionType;
    if (!getContainerElementsType(context, inputVariable->getType(), distributionType) || !getDistributionElementsType(context, distributionType, elementsType))
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
  friend class DiscretizeContainerOperatorClass;

  TypePtr elementsType;
  bool sampleBest;
};

class DiscretizeOperator : public ProxyFunction
{
public:
  DiscretizeOperator(bool sampleBest = true)
    : sampleBest(sampleBest) {}

  virtual FunctionPtr createImplementation(const std::vector<VariableSignaturePtr>& inputVariables) const
  {
    if (inputVariables.size() == 1 && inputVariables[0]->getType()->inheritsFrom(containerClass(distributionClass(anyType))))
      return new DiscretizeContainerOperator(sampleBest);
    return FunctionPtr();
  }

protected:
  friend class DiscretizeOperatorClass;
  bool sampleBest;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OPERATOR_DISCRETIZE_CONTAINER_H_
