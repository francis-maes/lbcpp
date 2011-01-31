/*-----------------------------------------.---------------------------------.
| Filename: DiscretizeContainerOperator.h  | Discretize Container Function   |
| Author  : Francis Maes                   |                                 |
| Started : 31/01/2011 22:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_OPERATOR_DISCRETIZE_CONTAINER_H_
# define LBCPP_FUNCTION_OPERATOR_DISCRETIZE_CONTAINER_H_

# include <lbcpp/Core/Function.h>
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Core/DynamicObject.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>

namespace lbcpp
{

class DiscretizeContainerOperator : public Function
{
public:
  DiscretizeContainerOperator(TypePtr elementsType, bool sampleBest)
    : elementsType(elementsType), sampleBest(sampleBest) {}
  DiscretizeContainerOperator() {}

  virtual TypePtr getInputType() const
    {return containerClass(distributionClass(elementsType));}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return vectorClass(elementsType);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    if (!checkType(context, input, containerClass(distributionClass(elementsType))) || !checkExistence(context, input))
      return Variable();

    const ContainerPtr& container = input.getObjectAndCast<Container>();

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

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_OPERATOR_DISCRETIZE_CONTAINER_H_
