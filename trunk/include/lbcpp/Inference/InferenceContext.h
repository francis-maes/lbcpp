/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.h             | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_H_
# define LBCPP_INFERENCE_CONTEXT_H_

# include "Inference.h"
# include <list>

namespace lbcpp
{

class Evaluator;
typedef ReferenceCountedObjectPtr<Evaluator> EvaluatorPtr;

class InferenceContext : public Object
{
public:
  typedef Inference::ReturnCode ReturnCode;

  virtual Variable run(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  ReturnCode train(InferencePtr inference, ContainerPtr examples);
  ReturnCode evaluate(InferencePtr inference, ContainerPtr examples, EvaluatorPtr evaluator);

  /*
  ** Inference Callbacks
  */
  void appendCallback(InferenceCallbackPtr callback);
  void removeCallback(InferenceCallbackPtr callback);
  void clearCallbacks();

protected:
  virtual InferenceStackPtr getCurrentStack() const = 0;

  friend class DecoratorInference;
  friend class SequentialInference;
  friend class ParallelInference;
  
  virtual Variable runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;
  
  Variable callRunInference(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  void callPreInference(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();
extern InferenceContextPtr multiThreadedInferenceContext(size_t numCpus);

class ThreadPool : public Object
{
public:
  ThreadPool(size_t numCpus = 1);
  virtual ~ThreadPool();

  void update();

  size_t getNumCpus() const
    {return numCpus;}

  size_t getNumWaitingThreads() const;
  size_t getNumRunningThreads() const;
  size_t getNumThreads() const;

  void addJob(juce::ThreadPoolJob* job, size_t priority = 0);
  void addJobAndWaitExecution(juce::ThreadPoolJob* job, size_t priority = 0);

  void waitThread(juce::Thread* thread);

private:
  size_t numCpus;
  size_t volatile numWaitingThreads;

  CriticalSection threadsLock;
  std::vector<juce::Thread* > threads;

  CriticalSection waitingJobsLock;
  std::vector< std::list< juce::ThreadPoolJob* > > waitingJobs;

  juce::ThreadPoolJob* popJob();
  void startThreadForJob(juce::ThreadPoolJob* job);
  juce::Thread* createThreadForJobIfAvailableCpu(juce::ThreadPoolJob* job);
};

typedef ReferenceCountedObjectPtr<ThreadPool> ThreadPoolPtr;


}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
