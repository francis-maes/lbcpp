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

// TODO : force abstract in xml ?
class OptimizerState : public Object, public FunctionCallback
{
public:
  OptimizerState(double autoSaveStateFrequency = 0);
  
  /** 
   * This function must be called before using the OptimizerState.
   * It is usefull if the Optimizer is "restarted" with an existing OptimizerState (i.e. in progress evaluations are lost).
   * This functions forces: totalNumberOfRequests = totalNumberOfResults
   */
  void initialize();
  /**
   * Save this OptimizerState in optimizerState.xml.
   * The saving operation is performed only if there is at least autoSaveStateFrequency seconds since the last saving operation.
   * @param force true value forces the saving operation even if there is less than autoSaveStateFrequency seconds since the last saving operation.
   */
  void autoSaveToFile(ExecutionContext& context, bool force = false);
  
  /*
  ** Requests
  */
  size_t getTotalNumberOfRequests() const;
  void incTotalNumberOfRequests();
  size_t getTotalNumberOfResults() const;
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
  
  CriticalSection lock; /**< Lock used to synchronize access to variables. */
    
  size_t totalNumberOfRequests;
  size_t totalNumberOfResults;
  
  Variable bestVariable;
  double bestScore;
    
  std::vector< std::pair<double, Variable> > processedRequests; /**< Buffer used to store results not handled yet by the Optimizer. */
  
  double autoSaveStateFrequency;    /**< Minimum delay between two save operations (in seconds). */
  double lastSaveTime;  /**< Timestamp of the last save operation. This variable is not serialized. */
};
  
typedef ReferenceCountedObjectPtr<OptimizerState> OptimizerStatePtr;
extern ClassPtr optimizerStateClass;

class SamplerBasedOptimizerState : public OptimizerState
{
public:
  SamplerBasedOptimizerState(const SamplerPtr& sampler, double autoSaveStateFrequency = 0)
    : OptimizerState(autoSaveStateFrequency), sampler(sampler), initialSampler(sampler) {}
  SamplerBasedOptimizerState() {}

  const SamplerPtr& getSampler() const
    {return sampler;}
  
  void setSampler(const SamplerPtr& newSampler)
    {sampler = newSampler;}
  
  /**
   * Prototype design pattern method.
   */
  SamplerPtr getCloneOfInitialSamplerInstance() const
    {return initialSampler->cloneAndCast<Sampler>();}
  
protected:
  friend class SamplerBasedOptimizerStateClass;

  SamplerPtr sampler;
  SamplerPtr initialSampler;  /**< Prototype design patter. */
};

typedef ReferenceCountedObjectPtr<SamplerBasedOptimizerState> SamplerBasedOptimizerStatePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_STATE_H_