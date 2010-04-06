/*-----------------------------------------.---------------------------------.
| Filename: IterativeClassificationVar....h| Iterative classification        |
| Author  : Francis Maes                   |                                 |
| Started : 29/03/2010 17:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VARIABLE_SET_MODEL_ITERATIVE_CLASSIFICATION_H_
# define LBCPP_VARIABLE_SET_MODEL_ITERATIVE_CLASSIFICATION_H_

# include "../VariableSetModel.h"

namespace lbcpp
{

class IterativeClassificationVariableSetModel : public VariableSetModel
{
public:
  IterativeClassificationVariableSetModel(ClassifierPtr initialClassifier, ClassifierPtr iterativeClassifier)
    : initialClassifier(initialClassifier), iterativeClassifier(iterativeClassifier) {}
  IterativeClassificationVariableSetModel() {}
  
  virtual void trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    classificationExamples = makeClassificationExamples(examples);
    VariableSetModel::trainBatch(examples, progress);
    classificationExamples = std::pair<ObjectContainerPtr, ObjectContainerPtr>();
  }

  enum {maxInferencePasses = 100};

  virtual void predict(VariableSetExamplePtr example, VariableSetPtr prediction) const
  {
    for (size_t i = 0; i < example->getNumVariables(); ++i)
      prediction->setVariable(i, initialClassifier->predict(example->getVariableFeatures(i)));
    
    std::vector<size_t> previousLabels;
    prediction->getVariables(previousLabels);
    jassert(previousLabels.size());
    
    for (size_t i = 0; i < maxInferencePasses; ++i)
    {
      // label each node in a randomly sampled order
      std::vector<size_t> order;
      RandomGenerator::getInstance().sampleOrder(0, example->getNumVariables(), order);
      for (size_t j = 0; j < order.size(); ++j)
      {
        size_t index = order[j];
        prediction->setVariable(index, iterativeClassifier->predict(example->getVariableFeatures(index, prediction)));
      }
      
      // stopping criterion
      std::vector<size_t> labels;
      prediction->getVariables(labels);
      jassert(labels.size() == previousLabels.size());
      size_t numCommon = 0;
      for (size_t j = 0; j < labels.size(); ++j)
        if (labels[j] == previousLabels[j])
          ++numCommon;
       
      double changeRatio = 1.0 - numCommon / (double)labels.size();
      std::cout << "." << std::flush;
      //std::cout << "ITERATION " << i << " NumCommon = " << numCommon << " changeRatio = " << changeRatio << std::endl;
      if (changeRatio < 0.0001)
        break;
      previousLabels = labels;
    }
    std::cout << std::endl;
  }
  
protected:
  std::pair<ObjectContainerPtr, ObjectContainerPtr> classificationExamples;
  
  ClassifierPtr initialClassifier;
  ClassifierPtr iterativeClassifier;
  
  std::pair<ObjectContainerPtr, ObjectContainerPtr> makeClassificationExamples(ObjectContainerPtr examples)
  {
    VectorObjectContainerPtr withoutContext = new VectorObjectContainer("ClassificationExample");
    VectorObjectContainerPtr withContext = new VectorObjectContainer("ClassificationExample");
    
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
        {
          withoutContext->append(new ClassificationExample(example->getVariableFeatures(j), value));
          withContext->append(new ClassificationExample(example->getVariableFeatures(j, targetVariables), value));
        }
      }
    }

    return std::make_pair(withoutContext, withContext);
  }
  
  virtual void trainBatchIteration(ObjectContainerPtr examples)
  {
    jassert(initialClassifier && iterativeClassifier);
    initialClassifier->trainStochastic(classificationExamples.first);
    iterativeClassifier->trainStochastic(classificationExamples.second);
  }  
};

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_SET_MODEL_ITERATIVE_CLASSIFICATION_H_
