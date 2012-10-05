/*-----------------------------------------.---------------------------------.
| Filename: LogLinearActionCodeSearchSampler.h | Search Sampler based on     |
| Author  : Francis Maes                   | action codes                    |
| Started : 05/10/2012 11:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_SAMPLER_SEARCH_LOG_LINEAR_ACTION_CODE_H_
# define LBCPP_ML_SAMPLER_SEARCH_LOG_LINEAR_ACTION_CODE_H_

# include <lbcpp-ml/Search.h>

namespace lbcpp
{

class LogLinearActionCodeSearchSampler : public SearchSampler
{
public:
  LogLinearActionCodeSearchSampler(double learningRate = 1.0)
    : learningRate(learningRate) {}

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    SearchTrajectoryPtr res(new SearchTrajectory());
    SearchStatePtr state = domain->createInitialState();
    while (!state->isFinalState())
    {
      ObjectPtr action = sampleAction(context, state);
      res->append(action);
      state->performTransition(context, action);
    }
    res->setFinalState(state);
    return res;
  }

  virtual bool isDeterministic() const // returns true if the sampler has became deterministic
    {return false;}

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {jassertfalse;} // FIXME

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
  {
    SearchTrajectoryPtr trajectory = object.staticCast<SearchTrajectory>();
    jassertfalse; // FIXME
  }

protected:
  friend class LogLinearActionCodeSearchSamplerClass;

  double learningRate;

  DenseDoubleVectorPtr parameters;

  double getParameter(size_t index) const
    {return parameters && parameters->getNumValues() > index ? parameters->getValue(index) : 0.0;}

  ObjectPtr sampleAction(ExecutionContext& context, SearchStatePtr state) const
  {
    DiscreteDomainPtr actionDomain = state->getActionsDomain().staticCast<DiscreteDomain>();
    size_t n = actionDomain->getNumElements();
    
    std::vector<double> probabilities(n, 0.0);
    double Z = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
      size_t code = domain->getActionCode(state, actionDomain->getElement(i));
      double p = exp(getParameter(code));
      probabilities[i] = p;
      Z += p;
    }

    size_t index = context.getRandomGenerator()->sampleWithProbabilities(probabilities, Z);
    return actionDomain->getElement(index);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_SAMPLER_SEARCH_LOG_LINEAR_ACTION_CODE_H_
