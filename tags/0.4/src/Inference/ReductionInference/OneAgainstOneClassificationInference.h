/*-----------------------------------------.---------------------------------.
| Filename: OneAgainstOneClassification...h| One vs. one classification      |
| Author  : Julien Becker                  |                                 |
| Started : 07/11/2010 19:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONE_AGAINST_ONE_CLASSIFICATION_H_
# define LBCPP_INFERENCE_ONE_AGAINST_ONE_CLASSIFICATION_H_

# include <lbcpp/Inference/ParallelInference.h>

namespace lbcpp
{

class OneAgainstOneClassificationInference : public VectorParallelInference
{
public:
  OneAgainstOneClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
    : VectorParallelInference(name), classes(classes), binaryClassifierModel(binaryClassifierModel)
  {
    checkInheritance(binaryClassifierModel->getOutputType(getInputType()), probabilityType);
    size_t n = classes->getNumElements();
    subInferences.resize(n * (n - 1) / 2);

    for (size_t index = 0, i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j, ++index)
      {
        InferencePtr subInference = binaryClassifierModel->cloneAndCast<Inference>();
        subInference->setName(classes->getElementName(i) + T(" vs ") + classes->getElementName(j));
        subInferences[index] = subInference;
      }
  }

  OneAgainstOneClassificationInference() {}

  virtual bool useMultiThreading() const
    {return false;}

  virtual TypePtr getInputType() const
    {return binaryClassifierModel->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return classes;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return classes;}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(subInferences.size());
    int correctClass = supervision.exists() ? supervision.getInteger() : -1;
    size_t n = classes->getNumElements();
    for (size_t index = 0, i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j, ++index)
      {
        if (correctClass >= 0 && ((size_t)correctClass == i || (size_t)correctClass == j))
          res->addSubInference(getSubInference(index), input, Variable(i == (size_t)correctClass));
        else
          res->addSubInference(getSubInference(index), input, Variable());
      }
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
  {
    size_t n = classes->getNumElements();
    std::vector<double> sumScores(n, 0.0);
    
    for (size_t index = 0, i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++n, ++index)
      {
        Variable prediction = state->getSubOutput(index);
        if (prediction.exists())
        {
          double p = prediction.getDouble();
          sumScores[i] += p;
          sumScores[j] += 1.0 - p;
        }
      }
    
    double bestScore = -DBL_MAX;
    int bestClass = -1;
    for (size_t i = 0; i < n; ++i)
      if (sumScores[i] > bestScore)
      {
        bestScore = sumScores[i];
        bestClass = i;
      }
    
    return bestClass >= 0 ? Variable(bestClass, classes) : Variable::missingValue(classes);
  }

private:
  friend class OneAgainstOneClassificationInferenceClass;

  EnumerationPtr classes;
  InferencePtr binaryClassifierModel;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONE_AGAINST_ONE_CLASSIFICATION_H_