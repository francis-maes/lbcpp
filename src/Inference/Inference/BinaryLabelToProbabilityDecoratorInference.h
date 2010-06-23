/*-----------------------------------------.---------------------------------.
| Filename: BinaryLabelToProbabilityDec...h| Transforms a binary label output|
| Author  : Francis Maes                   |  into a scalar probability      |
| Started : 28/05/2010 10:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_DECORATOR_BINARY_LABEL_TO_PROBABILITY_H_
# define LBCPP_INFERENCE_DECORATOR_BINARY_LABEL_TO_PROBABILITY_H_

# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class BinaryLabelToProbabilityDecoratorInference : public DecoratorInference
{
public:
  BinaryLabelToProbabilityDecoratorInference(const String& name, InferencePtr binaryClassifier, double temperature = 1.0)
    : DecoratorInference(name, binaryClassifier), temperature(temperature) {}
  BinaryLabelToProbabilityDecoratorInference() {}

  virtual ObjectPtr finalizeSubInference(ObjectPtr input, ObjectPtr supervision, ObjectPtr subInferenceOutput, ReturnCode& returnCode) const
  {
    if (!subInferenceOutput)
      return ObjectPtr();
    LabelPtr label = subInferenceOutput.dynamicCast<Label>();
    jassert(label && label->getDictionary() == BinaryClassificationDictionary::getInstance());
    double score = label->getIndex() == 1 ? label->getScore() : -label->getScore();
    double probability = 1.0 / (1.0 + exp(-score * temperature));
    return new Scalar(probability);
  }

protected:
  double temperature;

  virtual bool load(InputStream& istr)
    {return DecoratorInference::load(istr) && lbcpp::read(istr, temperature);}

  virtual void save(OutputStream& ostr) const
    {DecoratorInference::save(ostr); lbcpp::write(ostr, temperature);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_DECORATOR_BINARY_LABEL_TO_PROBABILITY_H_
