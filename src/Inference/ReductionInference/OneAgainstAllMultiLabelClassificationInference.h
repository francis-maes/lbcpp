/*-----------------------------------------.---------------------------------.
| Filename: OneAgainstAllMultiLabelClas...h| One vs. all multi-label         |
| Author  : Francis Maes                   |  classification                 |
| Started : 15/01/2011 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_ONE_AGAINST_ALL_MULTI_CLASSIFICATION_H_
# define LBCPP_INFERENCE_ONE_AGAINST_ALL_MULTI_CLASSIFICATION_H_

# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Core/DynamicObject.h>

namespace lbcpp
{

class OneAgainstAllMultiLabelClassificationInference : public VectorParallelInference
{
public:
  OneAgainstAllMultiLabelClassificationInference(const String& name, EnumerationPtr classes, InferencePtr binaryClassifierModel)
    : VectorParallelInference(name), classes(classes), binaryClassifierModel(binaryClassifierModel)
  {
    outputClass = enumBasedProbabilityVectorClass(classes);
    subInferences.resize(classes->getNumElements());
    for (size_t i = 0; i < subInferences.size(); ++i)
    {
      InferencePtr subInference = binaryClassifierModel->cloneAndCast<Inference>();
      subInference->setName(classes->getElementName(i));
      subInferences[i] = subInference;
    }
  }
  OneAgainstAllMultiLabelClassificationInference() {}

  virtual bool useMultiThreading() const
    {return false;}

  virtual TypePtr getInputType() const
    {return binaryClassifierModel->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return outputClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return outputClass;}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    res->reserve(subInferences.size());
    ObjectPtr supervisionObject = supervision.exists() ? supervision.getObject() : ObjectPtr();
    for (size_t i = 0; i < subInferences.size(); ++i)
      res->addSubInference(getSubInference(i), input, supervisionObject ? supervisionObject->getVariable(i) : Variable());
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
  {
    bool atLeastOnePrediction = false;
    ObjectPtr object = Object::create(outputClass);
    for (size_t i = 0; i < state->getNumSubInferences(); ++i)
    {
      const Variable& prediction = state->getSubOutput(i);
      if (prediction.exists())
      {
        object->setVariable(context, i, prediction);
        atLeastOnePrediction = true;
      }
    }
    return atLeastOnePrediction ? object : Variable::missingValue(outputClass);
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!VectorParallelInference::loadFromXml(importer))
      return false;
    outputClass = enumBasedProbabilityVectorClass(classes);
    return true;
  }

private:
  friend class OneAgainstAllMultiLabelClassificationInferenceClass;

  EnumerationPtr classes;
  InferencePtr binaryClassifierModel;
  ClassPtr outputClass;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_ONE_AGAINST_ALL_MULTI_CLASSIFICATION_H_
