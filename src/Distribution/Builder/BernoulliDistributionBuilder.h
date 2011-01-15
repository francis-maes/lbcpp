/*-----------------------------------------.---------------------------------.
| Filename: BernoulliProbabili...Builder.h | Bernoulli Probability           |
| Author  : Julien Becker                  | Distributions Builder           |
| Started : 14/01/2011 14:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BERNOULLI_PROBABILITY_DISTRIBUTION_BUILDER_H_
# define LBCPP_BERNOULLI_PROBABILITY_DISTRIBUTION_BUILDER_H_

# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/Distribution/DistributionBuilder.h>

namespace lbcpp
{
  
class BernoulliDistributionBuilder : public DistributionBuilder
{
public:
  BernoulliDistributionBuilder() : pTrue(0.0), count(0) {}
  
  virtual TypePtr getInputType() const
    {return booleanType;}
  
  virtual void clear()
  {
    pTrue = 0.0;
    count = 0;
  }
  
  virtual void addElement(const Variable& element, double weight)
  {
    if (!checkInheritance(element.getType(), booleanType))
      return;
    if (element.getBoolean())
      pTrue += 1.0;
    ++count;
  }
  
  virtual void addDistribution(const DistributionPtr& value, double weight)
  {
    BernoulliDistributionPtr valueDistribution = value.staticCast<BernoulliDistribution>();
    jassert(valueDistribution);
    pTrue += valueDistribution->getProbabilityOfTrue();
    ++count;
  }
  
  virtual DistributionPtr build(ExecutionContext& context) const
  {
    if (!count)
    {
      jassertfalse;
      return BernoulliDistributionPtr();
    }
    return new BernoulliDistribution(pTrue / count);
  }
  
protected:
  friend class BernoulliDistributionBuilderClass;
  
  double pTrue;
  size_t count;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_BERNOULLI_PROBABILITY_DISTRIBUTION_BUILDER_H_
