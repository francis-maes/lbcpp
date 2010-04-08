/*-----------------------------------------.---------------------------------.
| Filename: SharedParallelInferenceStep.h  | Reduction from a problem        |
| Author  : Francis Maes                   |   to a simpler problem          |
| Started : 08/04/2010 18:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
# define LBCPP_REDUCTION_PREDICTION_PROBLEM_H_

# include "InferenceStep.h"

namespace lbcpp
{

// Input: FeatureGenerator
// Output: FeatureVector
class ClassificationInferenceStep : public InferenceStep
{
public:
  ClassificationInferenceStep(const String& name)
    : InferenceStep(name) {}

  virtual ResultCode run(InferencePolicyPtr policy, ObjectPtr input, ObjectPtr& output) 
    {return policy->doClassification(classifier, input.dynamicCast<FeatureGenerator>(), *(FeatureGeneratorPtr* )&output);}

protected:
  ClassifierPtr classifier;
};


// Input: ObjectContainer
// Output: inherited from ObjectContainer
class SharedParallelInferenceStepBase : public InferenceStep
{
public:
  SharedParallelInferenceStepBase(const String& name, InferenceStepPtr subInference, const String& outputClassName)
    : InferenceStep(name), subInference(subInference), outputClassName(outputClassName) {}

  virtual ResultCode run(InferencePolicyPtr policy, ObjectPtr input, ObjectPtr& output)
  {
    ObjectContainerPtr inputContainer = input.dynamicCast<ObjectContainer>();
    std::vector< std::pair<InferenceStepPtr, ObjectPtr> > subInferences(inputContainer->size());
    for (size_t i = 0; i < subInferences.size(); ++i)
      subInferences[i] = std::make_pair(subInference, inputContainer->get(i));
    ObjectContainerPtr outputContainer = Object::createAndCast<ObjectContainer>(outputClassName);
    outputContainer->resize(inputContainer->size());
    return policy->doParallelSteps(subInferences, outputContainer);
  }

protected:
  String outputClassName;
  InferenceStepPtr subInference;
};

class SharedParallelInferenceStep : public SharedParallelInferenceStepBase
{
public:
  SharedParallelInferenceStep(const String& name, InferenceStepPtr subInference, const String& outputClassName)
    : SharedParallelInferenceStepBase(name, subInference, outputClassName) {}

  virtual size_t getNumSubObjects(ObjectPtr input, ObjectPtr output) const = 0;
  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const = 0;
  virtual ObjectPtr getSubOutput(ObjectPtr output, size_t index) const = 0;

  struct InputContainer : public ObjectContainer
  {
    InputContainer(ObjectPtr input, size_t numSubInputs, SharedParallelInferenceStep* pthis)
      : input(input), numSubInputs(numSubInputs), pthis(pthis) {}

    ObjectPtr input;
    size_t numSubInputs;
    SharedParallelInferenceStep* pthis;

    virtual size_t size() const
      {return numSubInputs;}

    virtual ObjectPtr get(size_t index) const
      {return pthis->getSubInput(input, index);}
  };

  struct OutputContainer : public ObjectContainer
  {
    OutputContainer(ObjectPtr output, size_t numSubOutput, SharedParallelInferenceStep* pthis)
      : output(output), numSubOutput(numSubOutput), pthis(pthis) {}

    ObjectPtr output;
    size_t numSubOutput;
    SharedParallelInferenceStep* pthis;

    virtual size_t size() const
      {return output;}

    virtual ObjectPtr get(size_t index) const
      {return pthis->getSubOutput(output, index);}
  };

  virtual ResultCode run(InferencePolicyPtr policy, ObjectPtr input, ObjectPtr& output)
  {
    size_t numObjects = getNumSubObjects(input, output);
    if (output)
      output = new OutputContainer(output, numObjects, this);
    input = new InputContainer(input, numObjects, this);
    return SharedParallelInferenceStepBase::run(policy, input, output);
  }
};

}; /* namespace lbcpp */

#endif //!LBCPP_REDUCTION_PREDICTION_PROBLEM_H_
