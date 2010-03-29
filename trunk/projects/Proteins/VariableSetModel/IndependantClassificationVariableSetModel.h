/*-----------------------------------------.---------------------------------.
| Filename: IndependantClassification....h | Independent classification      |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2010 17:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VARIABLE_SET_MODEL_INDEPENDANT_CLASSIFICATION_H_
# define LBCPP_VARIABLE_SET_MODEL_INDEPENDANT_CLASSIFICATION_H_

# include "../VariableSetModel.h"

namespace lbcpp
{

class IndependantClassificationVariableSetModel : public VariableSetModel
{
public:
  IndependantClassificationVariableSetModel(ClassifierPtr classifier = ClassifierPtr())
    : classifier(classifier) {}

  enum {maxInferencePasses = 100};
  
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    ObjectContainerPtr classificationExamples = makeClassificationExamples(examples);
    jassert(classifier);
    return classifier->trainBatch(classificationExamples, progress);
  }

  virtual void predict(VariableSetExamplePtr example, VariableSetPtr prediction) const
  {
    for (size_t i = 0; i < prediction->getNumVariables(); ++i)
      prediction->setVariable(i, classifier->predict(example->getVariableFeatures(i)));
  }

protected:
  ClassifierPtr classifier;
  
  ObjectContainerPtr makeClassificationExamples(ObjectContainerPtr examples)
  {
    VectorObjectContainerPtr res = new VectorObjectContainer("ClassificationExample");
    for (size_t i = 0; i < examples->size(); ++i)
    {
      VariableSetExamplePtr example = examples->getAndCast<VariableSetExample>(i);
      VariableSetPtr targetVariables = example->getTargetVariables();
      jassert(example);
      size_t n = example->getNumVariables();
      for (size_t j = 0; j < n; ++j)
      {
        size_t value;
        if (targetVariables->getVariable(j, value))
          res->append(new ClassificationExample(example->getVariableFeatures(j), value));
      }
    }
    return res;
  }  
};

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_SET_MODEL_INDEPENDANT_CLASSIFICATION_H_