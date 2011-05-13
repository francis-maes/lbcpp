/*-----------------------------------------.---------------------------------.
| Filename: SimpleResidueSampler.h         | SimpleResidueSampler            |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 7 mai 2011  15:04:24           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_

# define MAX_INTERVAL_VALUE 1000000

# include "precompiled.h"
# include "../Sampler.h"
# include "ParzenContinuousSampler.h"

namespace lbcpp
{

class SimpleResidueSampler;
typedef ReferenceCountedObjectPtr<SimpleResidueSampler> SimpleResidueSamplerPtr;

class SimpleResidueSampler : public CompositeSampler
{
public:
  SimpleResidueSampler()
    : CompositeSampler(), numResidues(1), residuesDeviation(0)
  {
  }

  SimpleResidueSampler(size_t numResidues, size_t residuesDeviation = 0)
    : CompositeSampler(1), numResidues(numResidues), residuesDeviation(residuesDeviation)
  {
    ParzenContinuousSamplerPtr temp = new ParzenContinuousSampler(0.0001, -1 * MAX_INTERVAL_VALUE,
        2 * MAX_INTERVAL_VALUE, 1.0 / 2.0, 0.5 * MAX_INTERVAL_VALUE, 0.25 * MAX_INTERVAL_VALUE);
    sons[0] = temp;
  }

  SimpleResidueSampler(const SimpleResidueSampler& sampler)
    : CompositeSampler(1), numResidues(sampler.numResidues),
      residuesDeviation(sampler.residuesDeviation)
  {
    ParzenContinuousSamplerPtr temp = new ParzenContinuousSampler(
        (*(sampler.sons[0].getObjectAndCast<ParzenContinuousSampler> ())));
    sons[0] = temp;
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    double rand = std::abs(
        sons[0].getObjectAndCast<Sampler> ()->sample(context, random, NULL).getDouble());
    rand = rand > (1 * MAX_INTERVAL_VALUE) ? (2 * MAX_INTERVAL_VALUE) - rand : rand;

    size_t residue = (size_t)std::floor(rand * (double)numResidues / (double)MAX_INTERVAL_VALUE);
    if (residue == numResidues)
      residue--;
    return Variable(residue);
  }

  /**
   * dataset = first : a Variable of integer type containing the residue observed.
   *           second : not yet used.
   */
  virtual void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    if ((dataset.size() < 2) || (numResidues <= 0))
      return;

    std::vector<std::pair<Variable, Variable> > data;
    double scaleFactor = (double)MAX_INTERVAL_VALUE / (double)numResidues;
    double varianceIncrement = (double)residuesDeviation * scaleFactor;

    for (size_t i = 0; i < dataset.size(); i++)
    {
      size_t res = (size_t)dataset[i].first.getInteger();
      double value = (double)res * scaleFactor;
      value = std::abs(value + varianceIncrement * random->sampleDoubleFromGaussian(0, 1));
      value = value > (1 * MAX_INTERVAL_VALUE) ? (2 * MAX_INTERVAL_VALUE) - value : value;
      data.push_back(std::pair<Variable, Variable>(Variable(value), Variable()));
    }

    sons[0].getObjectAndCast<Sampler> ()->learn(context, random, data);
  }

protected:
  friend class SimpleResidueSamplerClass;
  size_t numResidues;
  size_t residuesDeviation;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_
