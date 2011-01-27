
#ifndef LBCPP_PROTEIN_PARAMETERED_INFERENCE_FACTORY_H_
# define LBCPP_PROTEIN_PARAMETERED_INFERENCE_FACTORY_H_

# include "LearningParameter.h"
# include "Inference/ProteinInferenceFactory.h"
# include "Inference/ProteinInference.h"
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class ProteinTarget : public Object
{
public:
  ProteinTarget(ExecutionContext& context, const String& targets)
    {loadFromString(context, targets);}
  ProteinTarget() {}
  
  size_t getNumPasses() const
    {return tasks.size();}
  
  size_t getNumTasks(size_t passIndex) const
    {jassert(passIndex < getNumPasses()); return tasks[passIndex].size();}
  
  String getTask(size_t passIndex, size_t taskIndex) const
    {jassert(taskIndex < getNumTasks(passIndex)); return tasks[passIndex][taskIndex];}
  
  virtual String toString() const
    {return description;}

  virtual bool loadFromString(ExecutionContext& context, const String& str);
  
protected:
  friend class ProteinTargetClass;

  std::vector<std::vector<String> > tasks;
  String description;
};

typedef ReferenceCountedObjectPtr<ProteinTarget> ProteinTargetPtr;


class ParameteredProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  ParameteredProteinInferenceFactory(ExecutionContext& context, LearningParameterPtr defaultParameter = LearningParameterPtr());

  virtual void setParameter(const String& targetName, size_t stage, LearningParameterPtr parameter);
  virtual LearningParameterPtr getParameter(const String& targetName, size_t stage) const;
  virtual void setDefaultParameter(LearningParameterPtr defaultParameter);

  virtual InferencePtr createInference(ProteinTargetPtr proteinTarget) const;

protected:
  friend class ParameteredProteinInferenceFactoryClass;

  std::map<String, std::map<size_t, LearningParameterPtr> > learningParameters;
  LearningParameterPtr defaultParameter;
  size_t currentStage;
  
  virtual LearningParameterPtr getCurrentStageParameter(const String& targetName) const;
};

typedef ReferenceCountedObjectPtr<ParameteredProteinInferenceFactory> ParameteredProteinInferenceFactoryPtr;
  
class ExtraTreeProteinInferenceFactory : public ParameteredProteinInferenceFactory
{
public:
  ExtraTreeProteinInferenceFactory(ExecutionContext& context, ExtraTreeLearningParameterPtr defaultParameter);
  
  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const;
  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const;
  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const;
};

class NumericalProteinInferenceFactory : public ParameteredProteinInferenceFactory
{
public:
  NumericalProteinInferenceFactory(ExecutionContext& context);
  
  virtual PerceptionPtr createPerception(const String& targetName, PerceptionType type) const;
  virtual void getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const;
  
  virtual void setMaximumIterations(size_t maxIterations)
    {this->maxIterations = maxIterations;}
  
protected:  
  void addBiasInferenceIfNeeded(NumericalSupervisedInferencePtr inference, const String& targetName) const;
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName) const;
  
private:
  size_t maxIterations;
};

typedef ReferenceCountedObjectPtr<NumericalProteinInferenceFactory> NumericalProteinInferenceFactoryPtr;

class OneAgainstAllLinearSVMProteinInferenceFactory : public NumericalProteinInferenceFactory
{
public:
  OneAgainstAllLinearSVMProteinInferenceFactory(ExecutionContext& context);

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const;
  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const;
};

class MultiClassLinearSVMProteinInferenceFactory : public NumericalProteinInferenceFactory
{
public:
  MultiClassLinearSVMProteinInferenceFactory(ExecutionContext& context, bool forceMostViolated = false);

  virtual InferencePtr createBinaryClassifier(const String& targetName, PerceptionPtr perception) const;
  virtual InferencePtr createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const;
  
protected:
  bool forceMostViolated;
};

}; /* namespace */

#endif // !LBCPP_PROTEIN_PARAMETERED_INFERENCE_FACTORY_H_
