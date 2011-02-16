/*-----------------------------------------.---------------------------------.
| Filename: FrameBasedFunctionBatchLearner.h| FrameBasedFunction learner     |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:34               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_BATCH_LEARNER_FRAME_BASED_FUNCTION_H_
# define LBCPP_LEARNING_BATCH_LEARNER_FRAME_BASED_FUNCTION_H_

# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Core/Frame.h>

namespace lbcpp
{

class FrameBasedFunctionBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredExamplesType() const
    {return objectClass;} // frameObjectClass

  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const FrameBasedFunctionPtr& function = f.staticCast<FrameBasedFunction>();
    FrameClassPtr frameClass = function->getFrameClass();

    // make initial frames
    std::vector<FramePtr> trainingFrames;
    makeInitialFrames(trainingData, frameClass, trainingFrames);
    std::vector<FramePtr> validationFrames;
    if (validationData.size())
      makeInitialFrames(validationData, frameClass, validationFrames);

    // for each frame operator:
    for (size_t i = 0; i < frameClass->getNumMemberVariables(); ++i)
    {
      FrameOperatorSignaturePtr signature = frameClass->getMemberVariable(i).dynamicCast<FrameOperatorSignature>();
      if (signature)
      {
        if (signature->getFunction()->hasBatchLearner())
        {
          if (!learnSubFunction(context, frameClass, i, signature, trainingFrames, validationFrames))
            return false;
        }
        if (trainingFrames.size())
          computeSubFunction(i, trainingFrames);
        if (validationFrames.size())
          computeSubFunction(i, validationFrames);
      }
    }

    return true;
  }

protected:
  void makeInitialFrames(const std::vector<ObjectPtr>& data, const FrameClassPtr& frameClass, std::vector<FramePtr>& res) const
  {
    size_t n = data.size();
    res.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      FramePtr frame = new Frame(frameClass);
      size_t numInputVariables = data[i]->getNumVariables();
      jassert(numInputVariables < frame->getNumVariables());
      for (size_t j = 0; j < numInputVariables; ++j)
        frame->setVariable(j, data[i]->getVariable(j));
      res[i] = frame;
    }
  }

  ObjectVectorPtr makeSubInputs(const FrameClassPtr& frameClass, const std::vector<FramePtr>& frames, size_t variableIndex) const
  {
    FrameOperatorSignaturePtr signature = frameClass->getMemberVariable(variableIndex).staticCast<FrameOperatorSignature>();
    const FunctionPtr& function = signature->getFunction();
    const DynamicClassPtr& subInputsClass = function->getInputsClass();

    size_t n = frames.size();
    size_t numInputs = function->getNumInputs();
    jassert(numInputs == signature->getNumInputs());

    ObjectVectorPtr res = new ObjectVector(subInputsClass, n);
    std::vector<ObjectPtr>& v = res->getObjects();
    for (size_t i = 0; i < n; ++i)
    {
      const FramePtr& frame = frames[i];
      DenseGenericObjectPtr subInput(new DenseGenericObject(subInputsClass));
      for (size_t j = 0; j < numInputs; ++j)
        subInput->setVariable(j, frame->getOrComputeVariable(signature->getInputIndex(j)));
      v[i] = subInput;
    }
    return res;
  }

  bool learnSubFunction(ExecutionContext& context, const FrameClassPtr& frameClass, size_t variableIndex, const FrameOperatorSignaturePtr& signature, const std::vector<FramePtr>& trainingFrames, const std::vector<FramePtr>& validationFrames) const
  {
    const FunctionPtr& function = signature->getFunction();
    ObjectVectorPtr subTrainingData = makeSubInputs(frameClass, trainingFrames, variableIndex);
    ObjectVectorPtr subValidationData;
    if (validationFrames.size())
      subValidationData = makeSubInputs(frameClass, validationFrames, variableIndex);
    return function->train(context, subTrainingData, subValidationData);
  }

  void computeSubFunction(size_t variableIndex, std::vector<FramePtr>& frames) const
  {
    for (size_t i = 0; i < frames.size(); ++i)
      frames[i]->getOrComputeVariable(variableIndex);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_FRAME_BASED_FUNCTION_H_
