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
  
  virtual FeatureGeneratorPtr getFeatures(VariableSetExamplePtr example, size_t index) const
    {return example->getVariableFeatures(index);}

  virtual void trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    classificationExamples = makeClassificationExamples(examples);
    VariableSetModel::trainBatch(examples, progress);
    classificationExamples = ObjectContainerPtr();
  }

  virtual void predict(VariableSetExamplePtr example, VariableSetPtr prediction) const
  {
    for (size_t i = 0; i < prediction->getNumVariables(); ++i)
      prediction->setVariable(i, classifier->predict(getFeatures(example, i)));
  }

protected:
  ObjectContainerPtr classificationExamples;
  ClassifierPtr classifier;

  virtual void trainBatchIteration(ObjectContainerPtr examples)
  {
    jassert(classifier);
    // per-pass
    classifier->trainStochastic(classificationExamples->randomize());
    
    GradientBasedClassifierPtr gbc = classifier.dynamicCast<GradientBasedClassifier>();
    if (gbc)
      std::cout << "Parameters: L0 = " << gbc->getParameters()->l0norm() << " L1 = " << gbc->getParameters()->l1norm() << std::endl;
    
    // per-episode
/*    for (size_t i = 0; i < examples->size(); ++i)
    {
      VariableSetExamplePtr example = examples->getAndCast<VariableSetExample>(i);
      VectorObjectContainerPtr res = new VectorObjectContainer("ClassificationExample");
      makeClassificationExamples(example, res);
      classifier->trainStochastic(res->randomize());
    }*/
  }
  
  void makeClassificationExamples(VariableSetExamplePtr example, VectorObjectContainerPtr res)
  {
    jassert(example);
    VariableSetPtr variables = example->getTargetVariables();
    size_t n = variables->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      size_t value;
      if (variables->getVariable(i, value))
        res->append(new ClassificationExample(getFeatures(example, i), value));
    }
  }

  ObjectContainerPtr makeClassificationExamples(ObjectContainerPtr examples)
  {
    VectorObjectContainerPtr res = new VectorObjectContainer("ClassificationExample");
    for (size_t i = 0; i < examples->size(); ++i)
    {
      VariableSetExamplePtr example = examples->getAndCast<VariableSetExample>(i);
      makeClassificationExamples(example, res);
    }
    return res;
  }  
};

class OptimisticClassificationVariableSetModel : public IndependantClassificationVariableSetModel
{
public:
  OptimisticClassificationVariableSetModel(ClassifierPtr classifier = ClassifierPtr())
    : IndependantClassificationVariableSetModel(classifier) {}

  virtual FeatureGeneratorPtr getFeatures(VariableSetExamplePtr example, size_t index) const
    {return example->getVariableFeatures(index, example->getTargetVariables());}
};

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_SET_MODEL_INDEPENDANT_CLASSIFICATION_H_
