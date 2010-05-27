/*-----------------------------------------.---------------------------------.
| Filename: OneAgainstAllClassification...h| One vs. all classification      |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 16:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONE_AGAINST_ALL_CLASSIFICATION_H_
# define LBCPP_INFERENCE_ONE_AGAINST_ALL_CLASSIFICATION_H_

# include <lbcpp/Inference/InferenceBaseClasses.h>

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

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return input;}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
  {
    if (!supervision)
      return ObjectPtr();
    LabelPtr correctLabel = supervision.dynamicCast<Label>();
    jassert(correctLabel);
    return new Label(BinaryClassificationDictionary::getInstance(), correctLabel->getIndex() == index ? 1 : 0);
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return new DenseVector(dictionary);}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    if (!subOutput)
      return;
    DenseVectorPtr scoreVector = output.dynamicCast<DenseVector>();
    LabelPtr predictedLabel = subOutput.dynamicCast<Label>();
    jassert(scoreVector && predictedLabel);
    double score = predictedLabel->getScore();
    scoreVector->set(index, predictedLabel->getIndex() == 0 ? -score : score);
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectPtr res = ParallelInference::run(context, input, supervision, returnCode);
    if (!res)
      return ObjectPtr();
    DenseVectorPtr classScores = res.dynamicCast<DenseVector>();
    jassert(classScores);
    double bestScore = -DBL_MAX;
    int bestClass = -1;
    for (size_t i = 0; i < classScores->getNumValues(); ++i)
    {
      double score = classScores->get(i);
      if (score > bestScore)
        bestScore = score, bestClass = (int)i;
      else if (score == bestScore)
        bestClass = -1;
    }
    if (bestClass < 0)
      return ObjectPtr();
    return new Label(dictionary, (size_t)bestClass);
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
