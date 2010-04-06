/*-----------------------------------------.---------------------------------.
| Filename: SimulatedIterativeClassifi....h| SICA Algorithm                  |
| Author  : Francis Maes                   |                                 |
| Started : 30/03/2010 10:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_VARIABLE_SET_MODEL_SIMULATED_ITERATIVE_CLASSIFICATION_H_
# define LBCPP_VARIABLE_SET_MODEL_SIMULATED_ITERATIVE_CLASSIFICATION_H_

# include "../VariableSetModel.h"

namespace lbcpp
{

class SimulatedVariableSetModel : public VariableSetModel
{
public:
  SimulatedVariableSetModel(ClassifierPtr stochasticClassifier, bool deterministicLearning)
    : classifier(stochasticClassifier), deterministicLearning(deterministicLearning) {}
  SimulatedVariableSetModel() : deterministicLearning(false) {}
  
  virtual void predict(VariableSetExamplePtr example, VariableSetPtr prediction) const
    {const_cast<SimulatedVariableSetModel* >(this)->inferenceFunction(example, prediction, false);}

protected:
  ClassifierPtr classifier;
  StoppingCriterionPtr stoppingCriterion;
  bool deterministicLearning;
  
  virtual void inferenceFunction(VariableSetExamplePtr example, VariableSetPtr prediction, bool isTraining) = 0;
  
  VectorObjectContainerPtr classificationExamples;

  size_t classify(FeatureGeneratorPtr features, VariableSetExamplePtr example, size_t index, bool isTraining)
  {
    size_t correctOutput;
    if (isTraining && example->getTargetVariables()->getVariable(index, correctOutput))
    {
      // training
      size_t res = deterministicLearning ? classifier->predict(features) : classifier->sample(features);
      classificationExamples->append(new ClassificationExample(features, correctOutput));
      //classifier->trainStochasticExample(new ClassificationExample(features, correctOutput));
      return res;
    }
    else
    {
      // inference
      return classifier->predict(features);
    }
  }
  
  virtual void trainBatchIteration(ObjectContainerPtr examples)
  {
    classificationExamples = new VectorObjectContainer("ClassificationExample");
    performInferenceOnExamples(examples, true);
    std::cout << std::endl << classificationExamples->size() << " classification examples." << std::endl;
    classifier->trainStochastic(classificationExamples->randomize());
   
    GradientBasedClassifierPtr gbc = classifier.dynamicCast<GradientBasedClassifier>();
    if (gbc)
      std::cout << "Parameters: L0 = " << gbc->getParameters()->l0norm() << " L1 = " << gbc->getParameters()->l1norm() << std::endl;
   
  }
  
private:
  void performInferenceOnExamples(ObjectContainerPtr examples, bool isTraining)
  {
    for (size_t i = 0; i < examples->size(); ++i)
    {
      //if (isTraining)
      //  classifier->trainStochasticBegin();
      VariableSetExamplePtr example = examples->getAndCast<VariableSetExample>(i);
      jassert(example);
      VariableSetPtr prediction = example->createInitialPrediction();
      inferenceFunction(example, prediction, isTraining);
      //if (isTraining)
      //  classifier->trainStochasticEnd();
    }
  }
};

class SimulatedIterativeClassificationVariableSetModel : public SimulatedVariableSetModel
{
public:
  SimulatedIterativeClassificationVariableSetModel(ClassifierPtr stochasticClassifier = ClassifierPtr(),
                                                   size_t maxInferencePasses = 5,
                                                   bool randomOrderInference = true,
                                                   bool deterministicLearning = false)
    : SimulatedVariableSetModel(stochasticClassifier, deterministicLearning),
      dictionary(new FeatureDictionary(T("SICA"), StringDictionaryPtr(), new StringDictionary())),
      maxInferencePasses(maxInferencePasses), randomOrderInference(randomOrderInference) {}

protected:
  FeatureDictionaryPtr dictionary;
  size_t maxInferencePasses;
  bool randomOrderInference;

  virtual void inferenceFunction(VariableSetExamplePtr example, VariableSetPtr prediction, bool isTraining)
  {
    // initial predictions
    for (size_t i = 0; i < example->getNumVariables(); ++i)
    {
      FeatureGeneratorPtr features = subFeatureGenerator(dictionary, 0, example->getVariableFeatures(i));
      prediction->setVariable(i, classify(features, example, i, isTraining));
    }
    
    // iterative classification
    std::vector<size_t> previousLabels;
    prediction->getVariables(previousLabels);
    jassert(previousLabels.size());
    
    size_t numPasses = 0;
    std::vector<size_t> order(example->getNumVariables());
    if (!randomOrderInference)  // left to right order
      for (size_t i = 0; i < order.size(); ++i)
        order[i] = i;

    for (size_t i = 0; i < maxInferencePasses; ++i)
    {
      ++numPasses;
      
      if (randomOrderInference)
        RandomGenerator::getInstance().sampleOrder(0, example->getNumVariables(), order);

      // label each node
      for (size_t j = 0; j < order.size(); ++j)
      {
        size_t index = order[j];
        FeatureGeneratorPtr features = subFeatureGenerator(dictionary, i + 1, example->getVariableFeatures(index, prediction));
        prediction->setVariable(index, classify(features, example, index, isTraining));
      }
      
      // stopping criterion
      std::vector<size_t> labels;
      prediction->getVariables(labels);
      jassert(labels.size() == previousLabels.size());
      size_t numCommon = numCommonVariables(labels, previousLabels);
      double changeRatio = 1.0 - numCommon / (double)labels.size();
      //std::cout << "." << std::flush;
      //std::cout << "ITERATION " << i << " NumCommon = " << numCommon << " changeRatio = " << changeRatio << std::endl;
      if (changeRatio < 0.0001)
        break;
      previousLabels = labels;
    }
/*    {
      std::vector<size_t> target;
      example->getTargetVariables()->getVariables(target);
      std::cout << numPasses << " => " << numCommonVariables(target, previousLabels) << " / " << target.size() << "\t\t\t" << std::flush;
    }*/
    std::cout << "." << std::flush;
  }
  
private:
  static size_t numCommonVariables(const std::vector<size_t>& v1, const std::vector<size_t>& v2)
  {
    jassert(v1.size() == v2.size());
    size_t res = 0;
    for (size_t i = 0; i < v1.size(); ++i)
      if (v1[i] == v2[i])
        ++res;
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_SET_MODEL_SIMULATED_ITERATIVE_CLASSIFICATION_H_
