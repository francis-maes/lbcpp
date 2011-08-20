/*-----------------------------------------.---------------------------------.
| Filename: OptimizerState.h               | State associated with an        |
| Author  : Arnaud Schoofs                 | Optimizer (useful to restart    |
| Started : 04/04/2011                     | the Optimizer)                  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_STATE_H_
# define LBCPP_OPTIMIZER_STATE_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class OptimizerState : public Object
{
public:
  OptimizerState(const Variable& initialParameters = Variable(), double initialScore = DBL_MAX)
    : bestParameters(initialParameters), bestScore(initialScore) {}

  const Variable& getBestParameters() const
    {return bestParameters;}

  void setBestParameters(const Variable& bestParameters)
    {this->bestParameters = bestParameters;}

  double getBestScore() const
    {return bestScore;}

  void setBestScore(double bestScore)
    {this->bestScore = bestScore;}

protected:
  friend class OptimizerStateClass;

  Variable bestParameters;
  double bestScore;
};

typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;

extern ClassPtr optimizerStateClass;

class SamplerBasedOptimizerState : public OptimizerState
{
public:
  SamplerBasedOptimizerState(const SamplerPtr& sampler)
    : sampler(sampler), initialSampler(sampler), numIterations(0) {}

  const SamplerPtr& getSampler() const
    {return sampler;}
  
  void setSampler(const SamplerPtr& newSampler)
    {sampler = newSampler;}

  SamplerPtr getCloneOfInitialSamplerInstance() const
    {return initialSampler->cloneAndCast<Sampler>();}

  size_t getNumIterations() const
    {return numIterations;}

  void incrementNumIterations()
    {++numIterations;}

protected:
  friend class SamplerBasedOptimizerStateClass;

  SamplerPtr sampler;
  SamplerPtr initialSampler;  /**< Prototype design patter. */

  size_t numIterations;

  SamplerBasedOptimizerState() : numIterations(0) {}
};

typedef ReferenceCountedObjectPtr<SamplerBasedOptimizerState> SamplerBasedOptimizerStatePtr;

extern OptimizerStatePtr streamBasedOptimizerState(ExecutionContext& context, const ObjectPtr& initialState, const std::vector<StreamPtr>& streams);

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
