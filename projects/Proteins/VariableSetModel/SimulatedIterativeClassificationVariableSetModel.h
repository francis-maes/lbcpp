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
  SimulatedVariableSetModel(ClassifierPtr stochasticClassifier, StoppingCriterionPtr stoppingCriterion)
    : classifier(stochasticClassifier), stoppingCriterion(stoppingCriterion) {}
  SimulatedVariableSetModel() {}
  
  virtual bool trainBatch(ObjectContainerPtr examples, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    if (progress)
      progress->progressStart(T("Batch training with ") + lbcpp::toString(examples->size()) + T(" examples"));
    
    stoppingCriterion->reset();
    for (size_t iteration = 0; true; ++iteration)
    {
      std::cout << "Training..." << std::endl;
      performInferenceOnExamples(examples->randomize(), true);
      std::cout << std::endl;
      
      std::cout << "Evaluating..." << std::endl;
      double value = evaluate(examples);
      std::cout << std::endl;
      
      if (progress && !progress->progressStep(T("Batch training, accuracy = ") + lbcpp::toString(value), (double)(iteration + 1)))
        return false;
      if (stoppingCriterion->shouldOptimizerStop(value))
        break;
    }
    
    if (progress)
      progress->progressEnd();
    return true;
  }
  
  virtual void predict(VariableSetExamplePtr example, VariableSetPtr prediction) const
    {const_cast<SimulatedVariableSetModel* >(this)->inferenceFunction(example, prediction, false);}


protected:
  ClassifierPtr classifier;
  StoppingCriterionPtr stoppingCriterion;
  
  virtual void inferenceFunction(VariableSetExamplePtr example, VariableSetPtr prediction, bool isTraining) = 0;
  
  size_t classify(FeatureGeneratorPtr features, VariableSetExamplePtr example, size_t index, bool isTraining)
  {
    size_t correctOutput;
    if (isTraining && example->getTargetVariables()->getVariable(index, correctOutput))
    {
      // training
      size_t res = classifier->predict /*sample */(features);
      classifier->trainStochasticExample(new ClassificationExample(features, correctOutput));
      return res;
    }
    else
    {
      // inference
      return classifier->predict(features);
    }
  }
  
private:
  void performInferenceOnExamples(ObjectContainerPtr examples, bool isTraining)
  {
    for (size_t i = 0; i < examples->size(); ++i)
    {
      if (isTraining)
        classifier->trainStochasticBegin();
      VariableSetExamplePtr example = examples->getAndCast<VariableSetExample>(i);
      jassert(example);
      VariableSetPtr prediction = example->createInitialPrediction();
      inferenceFunction(example, prediction, isTraining);
      if (isTraining)
        classifier->trainStochasticEnd();
    }
  }
};

class SimulatedIterativeClassificationVariableSetModel : public SimulatedVariableSetModel
{
public:
  SimulatedIterativeClassificationVariableSetModel(ClassifierPtr stochasticClassifier, StoppingCriterionPtr stoppingCriterion)
    : SimulatedVariableSetModel(stochasticClassifier, stoppingCriterion),
      dictionary(new FeatureDictionary(T("SICA"), StringDictionaryPtr(), new StringDictionary())) {}
  SimulatedIterativeClassificationVariableSetModel() {}

  enum {maxInferencePasses = 10};
    
protected:
  FeatureDictionaryPtr dictionary;

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
    for (size_t i = 0; i < maxInferencePasses; ++i)
    {
      ++numPasses;
      
      // label each node in a randomly sampled order
      std::vector<size_t> order;
      RandomGenerator::getInstance().sampleOrder(0, example->getNumVariables(), order);
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
    
    {
      std::vector<size_t> target;
      example->getTargetVariables()->getVariables(target);
      std::cout << numPasses << " => " << numCommonVariables(target, previousLabels) << " / " << target.size() << "\t\t\t" << std::flush;
    }
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
