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

  /*
  ** Static analysis
  */
  virtual void accept(InferenceVisitorPtr visitor) = 0;

  /*
  ** Dynamic execution
  */
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

extern InferencePtr linearScalarInference(const String& name);
extern InferencePtr transferFunctionDecoratorInference(const String& name, InferencePtr decoratedInference, ScalarFunctionPtr transferFunction);

extern InferencePtr binaryLinearSVMInference(InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));
extern InferencePtr binaryLogisticRegressionInference(InferenceOnlineLearnerPtr learner, const String& name = T("unnamed"));

extern InferencePtr oneAgainstAllClassificationInference(const String& name, FeatureDictionaryPtr labelsDictionary, InferencePtr binaryClassifierModel);

extern InferencePtr runOnSupervisedExamplesInference(InferencePtr inference);
extern InferencePtr callbackBasedDecoratorInference(const String& name, InferencePtr decoratedInference, InferenceCallbackPtr callback);

}; /* namespace lbcpp */

#endif //!LBCPP_INFERENCE_STEP_H_
