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

  virtual FunctionPtr train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const FrameBasedFunctionPtr& function = f.staticCast<FrameBasedFunction>();
    FrameClassPtr frameClass = function->getFrameClass();

    // make initial frames
    std::vector<FramePtr> trainingFrames;
    makeInitialFrames(trainingData, trainingFrames);
    std::vector<FramePtr> validationFrames;
    if (validationData.size())
      makeInitialFrames(validationData, validationFrames);

    // for each frame operator:
    for (size_t i = 0; i < frameClass->getNumMemberVariables(); ++i)
    {
      FrameOperatorSignaturePtr signature = frameClass->getMemberVariable(i).dynamicCast<FrameOperatorSignature>();
      if (signature)
      {
        if (signature->getFunction()->hasBatchLearner())
          learnSubFunction(context, frameClass, i, signature, trainingFrames, validationFrames);
        if (trainingFrames.size())
          computeSubFunction(i, trainingFrames);
        if (validationFrames.size())
          computeSubFunction(i, validationFrames);
      }
    }

    return function;
  }

protected:
  void makeInitialFrames(const std::vector<ObjectPtr>& data, std::vector<FramePtr>& res) const
  {
    size_t n = data.size();
    res.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
      res[i] = data[i].staticCast<Frame>();
      jassert(res[i]);
    }
  }

  ObjectVectorPtr makeSubFrames(const FrameClassPtr& frameClass, const std::vector<FramePtr>& frames, size_t variableIndex) const
  {
    FrameOperatorSignaturePtr signature = frameClass->getMemberVariable(variableIndex).staticCast<FrameOperatorSignature>();
    const FunctionPtr& function = signature->getFunction();
    const FrameClassPtr& subFrameClass = function->getFrameClass();

    size_t n = frames.size();
    size_t numInputs = function->getNumInputs();
    jassert(numInputs == signature->getNumInputs());
    ObjectVectorPtr res = new ObjectVector(subFrameClass, n);

    std::vector<ObjectPtr>& subFrames = res->getObjects();
    for (size_t i = 0; i < n; ++i)
    {
      const FramePtr& frame = frames[i];
      FramePtr subFrame(new Frame(subFrameClass));
      for (size_t j = 0; j < numInputs; ++j)
        subFrame->setVariable(j, frame->getOrComputeVariable(signature->getInputIndex(j)));
      subFrames[i] = subFrame;
    }
    return res;
  }

  void learnSubFunction(ExecutionContext& context, const FrameClassPtr& frameClass, size_t variableIndex, const FrameOperatorSignaturePtr& signature, const std::vector<FramePtr>& trainingFrames, const std::vector<FramePtr>& validationFrames) const
  {
    const FunctionPtr& function = signature->getFunction();
    ObjectVectorPtr subTrainingData = makeSubFrames(frameClass, trainingFrames, variableIndex);
    ObjectVectorPtr subValidationData;
    if (validationFrames.size())
      subValidationData = makeSubFrames(frameClass, validationFrames, variableIndex);
    function->train(context, subTrainingData, subValidationData);
  }

  void computeSubFunction(size_t variableIndex, std::vector<FramePtr>& frames) const
  {
    for (size_t i = 0; i < frames.size(); ++i)
      frames[i]->getOrComputeVariable(variableIndex);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_BATCH_LEARNER_FRAME_BASED_FUNCTION_H_
