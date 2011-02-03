
#include "ParameteredProteinInferenceFactory.h"
#include <lbcpp/DecisionTree/DecisionTree.h>
#include <lbcpp/Function/StoppingCriterion.h>
#include <lbcpp/Optimizer/Optimizer.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>

using namespace lbcpp;

/*
** ProteinTarget
*/

bool ProteinTarget::loadFromString(ExecutionContext& context, const String& value)
{
  tasks.clear();
  description = value;
  
  for (int begin = 0; begin != -1 && begin < value.length(); )
  {
    int end = value.indexOfChar(begin, T(')'));
    if (value[begin] != T('(') || end == -1)
    {
      context.errorCallback(T("ProteinTarget::loadFromString"), value.quoted() + T(" is not a valid target expression"));
      return false;
    }
    
    StringArray taskValues;
    taskValues.addTokens(value.substring(begin + 1, end), T("-"), T(""));
    
    begin = value.indexOfChar(end, T('('));
    if (begin == -1)
      begin = value.length();
    
    Variable nbPass = Variable::createFromString(context, positiveIntegerType, value.substring(++end, begin));
    if (nbPass.isMissingValue())
    {
      context.errorCallback(T("ProteinTarget::loadFromString"), value.quoted() + T(" is not a valid target expression"));
      return false;
    }
    
    for (size_t n = 0; n < (size_t)nbPass.getInteger(); ++n)
    {
      std::vector<String> pass;
      for (size_t i = 0; i < (size_t)taskValues.size(); ++i)
      {
        if (taskValues[i] == T("SS3"))
          pass.push_back(T("secondaryStructure"));
        else if (taskValues[i] == T("SS8"))
          pass.push_back(T("dsspSecondaryStructure"));
        else if (taskValues[i] == T("SA"))
          pass.push_back(T("solventAccessibilityAt20p"));
        else if (taskValues[i] == T("DR"))
          pass.push_back(T("disorderRegions"));
        else if (taskValues[i] == T("StAl"))
          pass.push_back(T("structuralAlphabetSequence"));
        else
        {
          context.errorCallback(T("ProteinTarget::loadFromString"), taskValues[i].quoted() + T(" is not a valid task"));
          return false;
        }
      }
      
      tasks.push_back(pass);
    }
  }
  return true;
}

/*
** ParameteredProteinInferenceFactory
*/

ParameteredProteinInferenceFactory::ParameteredProteinInferenceFactory(ExecutionContext& context, LearningParameterPtr defaultParameter)
  : ProteinInferenceFactory(context), defaultParameter(defaultParameter), targetStageToOptimize(0) {}

void ParameteredProteinInferenceFactory::setParameter(const String& targetName, size_t stage, LearningParameterPtr parameter)
{
  learningParameters[targetName][stage] = parameter;
}

LearningParameterPtr ParameteredProteinInferenceFactory::getParameter(const String& targetName, size_t stage) const
{
  if (learningParameters.count(targetName) && learningParameters.find(targetName)->second.count(stage))
    return learningParameters.find(targetName)->second.find(stage)->second;
  return getDefaultParameter(targetName);
}

void ParameteredProteinInferenceFactory::setDefaultParameter(LearningParameterPtr defaultParameter)
{
  this->defaultParameter = defaultParameter;
}

LearningParameterPtr ParameteredProteinInferenceFactory::getDefaultParameter(const String& targetName) const
{
  if (targetName == T("secondaryStructure"))
    return new SecondaryStructureNumericalLearningParameter();
  jassertfalse;
  return defaultParameter;
}

void ParameteredProteinInferenceFactory::setTargetInferenceToOptimize(const String& targetName, size_t targetStage)
{
  targetNameToOptimize = targetName;
  targetStageToOptimize = targetStage;
}

InferencePtr ParameteredProteinInferenceFactory::createInference(ProteinTargetPtr proteinTarget) const
{
  ProteinSequentialInferencePtr inference = new ProteinSequentialInference("Main flow");
  for (size_t i = 0; i < proteinTarget->getNumPasses(); ++i)
  {
    ProteinParallelInferencePtr inferencePass = new ProteinParallelInference("Stage");
    const_cast<ParameteredProteinInferenceFactory*>(this)->currentStage = i;
    for (size_t j = 0; j < proteinTarget->getNumTasks(i); ++j)
    {
      StaticDecoratorInferencePtr step = createInferenceStep(proteinTarget->getTask(i, j));
      if (targetStageToOptimize == i && targetNameToOptimize == proteinTarget->getTask(i, j))
        createOptimizer(targetNameToOptimize, ((ProteinInferenceStepPtr)step)->getTargetInference());
      inferencePass->appendInference(step);
    }
    inference->appendInference(inferencePass);
  }
  return inference;
}

InferencePtr ParameteredProteinInferenceFactory::createOptimizer(const String& targetName, InferencePtr inference) const
{
  context.errorCallback(T("NumericalProteinInferenceFactory::createOptimizer"), T("Not yet implemented"));
  return InferencePtr();
}

LearningParameterPtr ParameteredProteinInferenceFactory::getCurrentStageParameter(const String& targetName) const
{
  return getParameter(targetName, currentStage);
}

/*
** ExtraTreeProteinInferenceFactory
*/

ExtraTreeProteinInferenceFactory::ExtraTreeProteinInferenceFactory(ExecutionContext& context, ExtraTreeLearningParameterPtr defaultParameter)
  : ParameteredProteinInferenceFactory(context, defaultParameter) {}

PerceptionPtr ExtraTreeProteinInferenceFactory::createPerception(const String& targetName, PerceptionType type) const
{
  PerceptionPtr res = ParameteredProteinInferenceFactory::createPerception(targetName, type);
  return res ? flattenPerception(res) : PerceptionPtr();
}

InferencePtr ExtraTreeProteinInferenceFactory::createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
{
  ExtraTreeLearningParameterPtr parameter = getCurrentStageParameter(targetName);
  return binaryClassificationExtraTreeInference(targetName, perception, parameter->getNumTrees(), parameter->getNumAttributeSamplesPerSplit(), parameter->getMinimumSizeForSplitting());
}

InferencePtr ExtraTreeProteinInferenceFactory::createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
{
  ExtraTreeLearningParameterPtr parameter = getCurrentStageParameter(targetName);
  return classificationExtraTreeInference(targetName, perception, classes, parameter->getNumTrees(), parameter->getNumAttributeSamplesPerSplit(), parameter->getMinimumSizeForSplitting());
}

/*
** NumericalProteinInferenceFactory
*/

NumericalProteinInferenceFactory::NumericalProteinInferenceFactory(ExecutionContext& context)
  : ParameteredProteinInferenceFactory(context), maxIterations(0) {}

PerceptionPtr NumericalProteinInferenceFactory::createPerception(const String& targetName, PerceptionType type) const
{
  PerceptionPtr res = ParameteredProteinInferenceFactory::createPerception(targetName, type);
  return res ? flattenPerception(res) : PerceptionPtr();
}

void NumericalProteinInferenceFactory::getPerceptionRewriteRules(PerceptionRewriterPtr rewriter) const
{
  rewriter->addRule(booleanType, booleanFeatures());
  rewriter->addRule(enumValueFeaturesPerceptionRewriteRule());
  rewriter->addRule(negativeLogProbabilityType, defaultPositiveDoubleFeatures(30, -3, 3));
  rewriter->addRule(probabilityType, defaultProbabilityFeatures());
  rewriter->addRule(positiveIntegerType, defaultPositiveIntegerFeatures());
  rewriter->addRule(integerType, defaultIntegerFeatures());
  
  // all other features
  rewriter->addRule(doubleType, identityPerception());
}

InferencePtr NumericalProteinInferenceFactory::createOptimizer(const String& targetName, InferencePtr inference) const
{
  OptimizerPtr optimizer = iterativeBracketingOptimizer(4, 2.0, uniformSampleAndPickBestOptimizer(7));
  NumericalLearningParameterPtr parameter = getCurrentStageParameter(targetName);
  IndependentMultiVariateDistributionPtr aprioriDistribution = parameter->getAprioriDistribution();
  InferencePtr autoTuneBatchLearner = autoTuneStochasticInferenceLearner(optimizer, aprioriDistribution, parameter);
  ((NumericalSupervisedInferencePtr)inference)->getSubInference()->setBatchLearner(precomputePerceptionsNumericalInferenceLearner(autoTuneBatchLearner));
  return inference;
}

void NumericalProteinInferenceFactory::addBiasInferenceIfNeeded(NumericalSupervisedInferencePtr inference, const String& targetName) const
{
  if (targetName.startsWith(T("contactMap"))
      || targetName == T("disorderRegions")
      || targetName == T("solventAccessibilityAt20p")
      || targetName == T("disulfideBonds"))
  {
    VectorSequentialInferencePtr sequentialInference = new VectorSequentialInference(targetName);
    sequentialInference->appendInference(inference->getSubInference());
    sequentialInference->appendInference(addBiasInference(targetName + T(" bias")));
    inference->setSubInference(sequentialInference);
  }
}

InferenceOnlineLearnerPtr NumericalProteinInferenceFactory::createOnlineLearner(const String& targetName) const
{
  jassert(!targetName.startsWith(T("contactMap"))); // Not yet implemented
  NumericalLearningParameterPtr parameter = getCurrentStageParameter(targetName);

  InferenceOnlineLearnerPtr res, lastLearner;
  /* randomizer */
  
  /* gradient */
  res = lastLearner = gradientDescentOnlineLearner(perStep, invLinearIterationFunction(parameter->getLearningRate(), parameter->getLearningRateDecrease()), true,
                                                   perStepMiniBatch20, l2RegularizerFunction(parameter->getRegularizer()));
  /* online evaluator */

  /* stopping criterion */
  StoppingCriterionPtr stoppingCriterion = logicalOr(maxIterationsStoppingCriterion(maxIterations), 
                                                     maxIterationsWithoutImprovementStoppingCriterion(2));
  lastLearner = lastLearner->setNextLearner(stoppingCriterionOnlineLearner(stoppingCriterion, true));
  
  return res;
}

/*
** NumericalProteinInferenceFactory - OneAgainstAllLinearSVMProteinInferenceFactory
*/

OneAgainstAllLinearSVMProteinInferenceFactory::OneAgainstAllLinearSVMProteinInferenceFactory(ExecutionContext& context)
  : NumericalProteinInferenceFactory(context) {}

InferencePtr OneAgainstAllLinearSVMProteinInferenceFactory::createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
{
  NumericalSupervisedInferencePtr res = binaryLinearSVMInference(targetName, perception);
  res->setStochasticLearner(createOnlineLearner(targetName));

  addBiasInferenceIfNeeded(res, targetName);
  return res;
}

InferencePtr OneAgainstAllLinearSVMProteinInferenceFactory::createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
{
  InferencePtr binaryClassifier = createBinaryClassifier(targetName, perception);
  return oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
}

/*
 ** NumericalProteinInferenceFactory - MultiClassLinearSVMProteinInferenceFactory
 */

MultiClassLinearSVMProteinInferenceFactory::MultiClassLinearSVMProteinInferenceFactory(ExecutionContext& context, bool forceMostViolated)
  : NumericalProteinInferenceFactory(context), forceMostViolated(forceMostViolated) {}

InferencePtr MultiClassLinearSVMProteinInferenceFactory::createBinaryClassifier(const String& targetName, PerceptionPtr perception) const
{
  NumericalSupervisedInferencePtr res = binaryLinearSVMInference(targetName, perception);
  res->setStochasticLearner(createOnlineLearner(targetName));
  
  addBiasInferenceIfNeeded(res, targetName);
  return res;
}

InferencePtr MultiClassLinearSVMProteinInferenceFactory::createMultiClassClassifier(const String& targetName, PerceptionPtr perception, EnumerationPtr classes) const
{
  NumericalSupervisedInferencePtr res = multiClassLinearSVMInference(targetName, perception, classes, (forceMostViolated || targetName == T("structuralAlphabet")));
  return res;
}
