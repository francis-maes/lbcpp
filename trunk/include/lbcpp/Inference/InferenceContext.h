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

  void callPreInference(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode);
  void callPostInference(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode);

protected:
  friend class DecoratorInference;
  friend class SequentialInference;
  friend class ParallelInference;
  
  virtual void preInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;
  virtual void postInference(InferencePtr inference, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode) = 0;

  virtual Variable runDecoratorInference(DecoratorInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runSequentialInference(SequentialInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode) = 0;
  
  Variable callRunInference(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode);
    
private:
  std::vector<InferenceCallbackPtr> callbacks;
};

extern InferenceContextPtr singleThreadedInferenceContext();

class Job : public NameableObject
{
public:
  Job(const String& name)
    : NameableObject(name), jobShouldExit(false) {}
  Job() : jobShouldExit(false) {}

  virtual String getCurrentStatus() const = 0;

  virtual bool runJob(String& failureReason) = 0;

  bool shouldExit() const
    {return jobShouldExit;}

  void signalJobShouldExit()
    {jobShouldExit = true;}

private:
  friend class JobClass;

  bool volatile jobShouldExit;
};

typedef ReferenceCountedObjectPtr<Job> JobPtr;

class ThreadPool : public Object
{
public:
  ThreadPool(size_t numCpus = 1, bool verbose = false);
  virtual ~ThreadPool();

  void update();

  size_t getNumCpus() const
    {return numCpus;}

  size_t getNumFreeCpus() const
    {return juce::jmax(0, (int)getNumCpus() - (int)getNumRunningThreads());}

  size_t getNumWaitingThreads() const;
  size_t getNumRunningThreads() const;
  size_t getNumThreads() const;

  void addJob(JobPtr job, size_t priority = 0);
  void addJobAndWaitExecution(JobPtr job, size_t priority = 0);

  void waitThread(Thread* thread);
  bool isThreadWaiting(Thread* thread) const;

  void writeCurrentState(std::ostream& ostr);

private:
  size_t numCpus;
  bool verbose;

  CriticalSection threadsLock;
  std::vector<juce::Thread* > threads;

  CriticalSection waitingThreadsLock;
  std::set<juce::Thread* > waitingThreads;

  CriticalSection waitingJobsLock;
  std::vector< std::list< JobPtr > > waitingJobs;

  JobPtr popJob();
  void startThreadForJob(JobPtr job);
  juce::Thread* createThreadForJobIfAvailableCpu(JobPtr job);
};

typedef ReferenceCountedObjectPtr<ThreadPool> ThreadPoolPtr;

extern InferenceContextPtr multiThreadedInferenceContext(ThreadPoolPtr threadPool);

inline InferenceContextPtr multiThreadedInferenceContext(size_t numCpus)
  {return multiThreadedInferenceContext(new ThreadPool(numCpus));}

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_CONTEXT_H_
