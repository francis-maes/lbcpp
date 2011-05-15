/*-----------------------------------------.---------------------------------.
| Filename: OptimizerState.h               | State associated with an        |
| Author  : Arnaud Schoofs                 | Optimizer (useful to restart    |
| Started : 04/04/2011                     | the Optimizer)                  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_STATE_H_
# define LBCPP_OPTIMIZER_STATE_H_

# include <lbcpp/Sampler/Sampler.h> // new 
# include <lbcpp/Distribution/Distribution.h> // old
# include <lbcpp/Core/Function.h>
# include <lbcpp/Optimizer/OptimizerContext.h>

namespace lbcpp
{

// TODO arnaud : force abstract in xml ?
class OptimizerState : public Object, public FunctionCallback
{
public:
  // args in seconds -> converted into ms
  OptimizerState(size_t autoSaveStateFrequency = 0);

  void initialize();
  void autoSaveToFile(ExecutionContext& context, bool force = false)
  {
    ScopedLock _(lock);
    if (force || (autoSaveStateFrequency && Time::currentTimeMillis() - lastSaveTime >= autoSaveStateFrequency)) {
      if (File::getCurrentWorkingDirectory().getChildFile(T("optimizerState.xml")).existsAsFile())
        File::getCurrentWorkingDirectory().getChildFile(T("optimizerState.xml")).copyFileTo(File::getCurrentWorkingDirectory().getChildFile(T("optimizerState_backup.xml")));
      saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(T("optimizerState.xml")));  // TODO arnaud : file name as args ?
      lastSaveTime = Time::currentTimeMillis();
    }
  }
  
  /*
  ** Requests
  */
  size_t getTotalNumberOfRequests() const;
  void incTotalNumberOfRequests();
  size_t getTotalNumberOfEvaluations() const;
  size_t getNumberOfInProgressEvaluations() const;
  
  /*
  ** Processeded requests
  */
  size_t getNumberOfProcessedRequests() const;
  const std::vector< std::pair<double, Variable> >& getProcessedRequests() const;
  void flushProcessedRequests();
  void flushFirstProcessedRequests(size_t nb);
  void flushLastProcessedRequests(size_t nb);

  
  /*
  ** Best variable and score
  */
  const Variable& getBestVariable() const;
  void setBestVariable(const Variable& variable);
  double getBestScore() const;
  void setBestScore(double score);
  
  /*
  ** Critical Section
  */
  const CriticalSection& getLock() const;
  
  /*
  ** FunctionCallback
  */
  virtual void functionReturned(ExecutionContext& context, const FunctionPtr& function, const Variable* inputs, const Variable& output);
  
protected:  
  friend class OptimizerStateClass;
  
  CriticalSection lock;
    
  size_t totalNumberOfRequests;
  size_t totalNumberOfEvaluations;
  
  Variable bestVariable;
  double bestScore;
    
  std::vector< std::pair<double, Variable> > processedRequests;  // evaluated WUs not processed yet
  
  juce::int64 autoSaveStateFrequency; // in milliseconds
  juce::int64 lastSaveTime; // timestamp in ms
};
  
typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;
extern ClassPtr optimizerStateClass;

class DistributionBasedOptimizerState : public OptimizerState
{
public:
  DistributionBasedOptimizerState(size_t autoSaveStateFrequency = 0) : OptimizerState(autoSaveStateFrequency) {}
  
  const DistributionPtr& getDistribution() const
    {return distribution;}
  
  void setDistribution(const DistributionPtr& newDistribution)
    {distribution = newDistribution;}
  
protected:  
  friend class DistributionBasedOptimizerStateClass;
  
  DistributionPtr distribution;
};

typedef ReferenceCountedObjectPtr<DistributionBasedOptimizerState> DistributionBasedOptimizerStatePtr;

class SamplerBasedOptimizerState : public OptimizerState
{
public:
  SamplerBasedOptimizerState(const SamplerPtr& sampler, size_t autoSaveStateFrequency = 0)
    : OptimizerState(autoSaveStateFrequency), sampler(sampler) {}
  SamplerBasedOptimizerState() {}

  const SamplerPtr& getSampler() const
    {return sampler;}
  
  void setSampler(const SamplerPtr& newSampler)
    {sampler = newSampler;}
  
protected:
  friend class SamplerBasedOptimizerStateClass;

  SamplerPtr sampler;
};

typedef ReferenceCountedObjectPtr<SamplerBasedOptimizerState> SamplerBasedOptimizerStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_
