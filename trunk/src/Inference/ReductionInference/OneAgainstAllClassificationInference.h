/*-----------------------------------------.---------------------------------.
| Filename: OneAgainstAllClassification...h| One vs. all classification      |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 16:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONE_AGAINST_ALL_CLASSIFICATION_H_
# define LBCPP_INFERENCE_ONE_AGAINST_ALL_CLASSIFICATION_H_

# include <lbcpp/Inference/ParallelInference.h>

namespace lbcpp
{

class OneAgainstAllClassificationInference : public VectorParallelInference
{
public:
  OneAgainstAllClassificationInference(ExecutionContext& context, const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
    : VectorParallelInference(name), classes(classes), binaryClassifierModel(binaryClassifierModel)
  {
    subInferences.resize(classes->getNumElements());
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      InferencePtr subInference = binaryClassifierModel->cloneAndCast<Inference>(context);
      subInference->setName(classes->getElementName(i));
      subInferences[i] = subInference;
    }
  }
  OneAgainstAllClassificationInference() {}

  virtual bool useMultiThreading() const
    {return false;}

  virtual TypePtr getInputType() const
    {return binaryClassifierModel->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return classes;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return classes;}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContext& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(subInferences.size());
    int correctClass = supervision.exists() ? supervision.getInteger() : -1;
    for (size_t i = 0; i < subInferences.size(); ++i)
      res->addSubInference(getSubInference(i), input, correctClass >= 0 ? Variable(i == (size_t)correctClass) : Variable());
    return res;
  }

  virtual Variable finalizeInference(InferenceContext& context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    double bestScore = -DBL_MAX;
    int bestClass = -1;
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      Variable prediction = state->getSubOutput(i);
      if (!prediction.exists())
        continue;

      double score = prediction.getDouble();
      if (score > bestScore)
        bestScore = score, bestClass = (int)i;
      else if (score == bestScore)
        bestClass = -1;
    }
    if (bestClass < 0)
      return Variable();
    return Variable(bestClass, classes);
  }

private:
  friend class OneAgainstAllClassificationInferenceClass;

  EnumerationPtr classes;
  InferencePtr binaryClassifierModel;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONE_AGAINST_ALL_CLASSIFICATION_H_
