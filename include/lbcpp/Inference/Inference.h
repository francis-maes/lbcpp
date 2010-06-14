/*-----------------------------------------.---------------------------------.
| Filename: Inference.h                    | Inference base class            |
| Author  : Francis Maes                   |                                 |
| Started : 08/04/2010 19:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_STEP_H_
# define LBCPP_INFERENCE_STEP_H_

# include "predeclarations.h"
# include "../ObjectPredeclarations.h"

namespace lbcpp
{

class Inference : public NameableObject
{
public:
  Inference(const String& name = T("Unnamed"))
    : NameableObject(name) {}

  static InferencePtr createFromFile(const File& file)
    {return Object::createFromFileAndCast<Inference>(file);}
  
  enum ReturnCode
  {
    finishedReturnCode = 0,
    canceledReturnCode,
    errorReturnCode,
  };

  // Used in SharedParallelInference before and after a block of many run() calls
  virtual void beginRunSession() {}
  virtual void endRunSession() {}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode) = 0;

  /*
  ** Learner
  */
  InferenceOnlineLearnerPtr getLearner() const
    {return learner;}

  void setLearner(InferenceOnlineLearnerPtr learner)
    {this->learner = learner;}

protected:
  friend class InferenceContext;

  InferenceOnlineLearnerPtr learner;

  // todo: save learner
};

// Atomic
extern InferencePtr linearScalarInference(const String& name);
extern InferencePtr transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction);

// Binary Classification
extern InferencePtr binaryLinearSVMInference(InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr binaryLogisticRegressionInference(InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr binaryLabelToProbabilityInference(const String& name, InferencePtr binaryClassifier, double temperature = 1.0);

// Regression
extern InferencePtr squareRegressionInference(InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr absoluteRegressionInference(InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr dihedralAngleRegressionInference(InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

// MultiClass Classification
extern InferencePtr oneAgainstAllClassificationInference(const String& name, FeatureDictionaryPtr labelsDictionary, InferencePtr binaryClassifierModel);

// Misc
extern InferencePtr runOnSupervisedExamplesInference(InferencePtr inference);
extern InferencePtr callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback);

class InferenceVector
{
public:
  size_t size() const
    {return v.size();}

  void resize(size_t size)
    {v.resize(size);}

  void set(size_t index, InferencePtr subInference)
    {jassert(index < v.size()); v[index] = subInference;}

  InferencePtr get(size_t index) const
    {jassert(index < v.size()); return v[index];}

  InferencePtr operator [](size_t index) const
    {return get(index);}

  void append(InferencePtr inference)
    {v.push_back(inference);}

  int find(InferencePtr inference) const;

  bool saveToDirectory(const File& file) const;
  bool loadFromDirectory(const File& file);

  File getSubInferenceFile(size_t index, const File& directory) const;

protected:
  std::vector<InferencePtr> v;
};

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
