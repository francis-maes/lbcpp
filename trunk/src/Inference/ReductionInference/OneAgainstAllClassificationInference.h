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
  OneAgainstAllClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
    : VectorStaticParallelInference(name), classes(classes), binaryClassifierModel(binaryClassifierModel)
  {
    subInferences.resize(classes->getNumElements());
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      InferencePtr subInference = binaryClassifierModel->cloneAndCast<BinaryClassificationInference>();
      subInference->setName(classes->getElementName(i));
      subInferences.set(i, subInference);
    }
  }
  OneAgainstAllClassificationInference() {}

  virtual TypePtr getInputType() const
    {return binaryClassifierModel->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return classes;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return classes;}

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(subInferences.size());

    int correctClass = -1;
    if (supervision)
    {
      if (supervision.isInteger())
        correctClass = supervision.getInteger();
      else if (supervision.isObject())
      {
        LabelPtr label = supervision.dynamicCast<Label>();
        jassert(label);
        correctClass = (int)label->getIndex();
      }
      else
        jassert(false);
    }
    for (size_t i = 0; i < subInferences.size(); ++i)
      res->addSubInference(subInferences.get(i), input, correctClass >= 0 ? Variable(i == (size_t)correctClass) : Variable());
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
    return Variable(bestClass, classes);
  }

private:
  EnumerationPtr classes;
  InferencePtr binaryClassifierModel;

  virtual bool load(InputStream& istr)
  {
    String classesName;
    if (!VectorStaticParallelInference::load(istr) || !lbcpp::read(istr, classesName))
      return false;
    return (classes = Enumeration::get(classesName)) != EnumerationPtr();
  }

  virtual void save(OutputStream& ostr) const
    {VectorStaticParallelInference::save(ostr); lbcpp::write(ostr, classes->getName());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONE_AGAINST_ALL_CLASSIFICATION_H_
