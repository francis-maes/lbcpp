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

class OneAgainstAllClassificationInference : public VectorStaticParallelInference
{
public:
  OneAgainstAllClassificationInference(const String& name, FeatureDictionaryPtr labelsDictionary, InferencePtr binaryClassifierModel)
    : VectorStaticParallelInference(name), dictionary(labelsDictionary)
  {
    subInferences.resize(dictionary->getNumFeatures());
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      InferencePtr subInference = binaryClassifierModel->cloneAndCast<BinaryClassificationInference>();
      subInference->setName(dictionary->getFeature(i));
      subInferences.set(i, subInference);
    }
  }
  OneAgainstAllClassificationInference() {}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(subInferences.size());

    if (supervision)
    {
      LabelPtr correctLabel = supervision.dynamicCast<Label>();
      jassert(correctLabel);
      size_t correct = correctLabel->getIndex();
      for (size_t i = 0; i < subInferences.size(); ++i)
        res->addSubInference(subInferences.get(i), input,
            LabelPtr(new Label(BinaryClassificationDictionary::getInstance(), i == correct ? 1 : 0)));
    }
    else
      for (size_t i = 0; i < subInferences.size(); ++i)
        res->addSubInference(subInferences.get(i), input, Variable());

    return res;
  }

  virtual Variable finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    double bestScore = -DBL_MAX;
    int bestClass = -1;
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      Variable prediction = state->getSubOutput(i);
      if (!prediction)
        continue;

      double score = prediction.getDouble();
      if (score > bestScore)
        bestScore = score, bestClass = (int)i;
      else if (score == bestScore)
        bestClass = -1;
    }
    if (bestClass < 0)
      return Variable();
    return LabelPtr(new Label(dictionary, (size_t)bestClass));
  }

private:
  FeatureDictionaryPtr dictionary;

  virtual bool load(InputStream& istr)
  {
    if (!VectorStaticParallelInference::load(istr))
      return false;
    dictionary = FeatureDictionaryManager::getInstance().readDictionaryNameAndGet(istr);
    return dictionary != FeatureDictionaryPtr();
  }

  virtual void save(OutputStream& ostr) const
    {VectorStaticParallelInference::save(ostr); lbcpp::write(ostr, dictionary->getName());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONE_AGAINST_ALL_CLASSIFICATION_H_
