/*-----------------------------------------.---------------------------------.
| Filename: NumericalProteinPredictorPar..h| Numerical Protein Predictor     |
| Author  : Francis Maes                   |  Parameters                     |
| Started : 01/03/2011 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PREDICTOR_NUMERICAL_PARAMETERS_H_
# define LBCPP_PROTEINS_PREDICTOR_NUMERICAL_PARAMETERS_H_

# include "ProteinPredictorParameters.h"
# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

class NumericalProteinPredictorParameters : public ProteinPredictorParameters
{
public:
  NumericalProteinPredictorParameters(NumericalProteinFeaturesParametersPtr featuresParameters, LearnerParametersPtr learningParameters)
    : featuresParameters(featuresParameters), learningParameters(learningParameters) {}

  NumericalProteinPredictorParameters()
    : featuresParameters(new NumericalProteinFeaturesParameters()),
      learningParameters(new StochasticGDParameters(constantIterationFunction(0.1))) {}

  // Features
  void primaryResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    builder.addInput(positiveIntegerType, T("position"));
    builder.addInput(proteinClass, T("protein"));
    size_t aminoAcid = builder.addFunction(getElementInVariableFunction(T("primaryStructure")), 1, 0);
    size_t pssmRow = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), 1, 0);
    size_t ss3 = builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), 1, 0);
    size_t ss8 = builder.addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), 1, 0);
    size_t stal = builder.addFunction(getElementInVariableFunction(T("structuralAlphabetSequence")), 1, 0);
    size_t sa20 = builder.addFunction(getElementInVariableFunction(T("solventAccessibilityAt20p")), 1, 0);
    size_t dr = builder.addFunction(getElementInVariableFunction(T("disorderRegions")), 1, 0);

    // feature generators
    builder.startSelection();
    
      builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));
      builder.addFunction(enumerationDistributionFeatureGenerator(), pssmRow, T("pssm"));
      builder.addFunction(enumerationDistributionFeatureGenerator(), ss3, T("ss3"));
      builder.addFunction(enumerationDistributionFeatureGenerator(), ss8, T("ss8"));
      builder.addFunction(enumerationDistributionFeatureGenerator(), stal, T("stal"));
      builder.addFunction(defaultProbabilityFeatureGenerator(10), sa20, T("sa20"));
      builder.addFunction(defaultProbabilityFeatureGenerator(10), dr, T("dr"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
  }

  void primaryResidueFeaturesVector(CompositeFunctionBuilder& builder) const
  {
    size_t input = builder.addInput(proteinClass, T("protein"));
    size_t length = builder.addFunction(proteinLengthFunction(), 0);
    builder.addFunction(createVectorFunction(function(&NumericalProteinPredictorParameters::primaryResidueFeatures)), length, input);
  }

  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);
    size_t primaryResidueFeatures = builder.addInput(vectorClass(doubleVectorClass()));
    size_t primaryResidueFeaturesAcc = builder.addInput(containerClass());
    size_t globalFeatures = builder.addInput(doubleVectorClass());
    
    builder.startSelection();

      if (featuresParameters->residueGlobalFeatures)
        builder.addInSelection(globalFeatures);

      if (featuresParameters->residueWindowSize)
        builder.addFunction(windowFeatureGenerator(featuresParameters->residueWindowSize), primaryResidueFeatures, position, T("window"));

      if (featuresParameters->residueLocalMeanSize)
        builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residueLocalMeanSize), primaryResidueFeaturesAcc, position, T("mean") + String((int)featuresParameters->residueLocalMeanSize));

      if (featuresParameters->residueMediumMeanSize)
        builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residueMediumMeanSize), primaryResidueFeaturesAcc, position, T("mean") + String((int)featuresParameters->residueMediumMeanSize));
     
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  void residueFeaturesVector(CompositeFunctionBuilder& builder) const
  {
    size_t protein = builder.addInput(proteinClass, T("protein"));

    builder.startSelection();

      builder.addFunction(proteinLengthFunction(), protein);
      size_t primaryFeatures = builder.addFunction(function(&NumericalProteinPredictorParameters::primaryResidueFeaturesVector), protein);
      size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("globalmean"));

    builder.finishSelectionWithFunction(createVectorFunction(function(&NumericalProteinPredictorParameters::residueFeatures)), T("residueFeatureVectors"));
  }

  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
    {residueFeaturesVector(builder);}

  typedef void (NumericalProteinPredictorParameters::*ThisClassFunctionBuildFunction)(CompositeFunctionBuilder& builder) const; 

  FunctionPtr function(ThisClassFunctionBuildFunction buildFunc) const
    {return function((FunctionBuildFunction)buildFunc);}

  FunctionPtr function(FunctionBuildFunction buildFunc) const
    {return new MethodBasedCompositeFunction(refCountedPointerFromThis(this), buildFunc);}

  // Learning Machine
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    StochasticGDParametersPtr parameters = this->learningParameters->cloneAndCast<StochasticGDParameters>();

    switch (target)
    {
    case drTarget:
      parameters->setEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore));
      return linearBinaryClassifier(parameters, true, binaryClassificationMCCScore);

    case sa20Target:
      parameters->setEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore));
      return linearBinaryClassifier(parameters, true, binaryClassificationAccuracyScore);

    default:
      parameters->setEvaluator(defaultSupervisedEvaluator());
      return linearLearningMachine(parameters);
    };
  }

protected:
  friend class NumericalProteinPredictorParametersClass;

  NumericalProteinFeaturesParametersPtr featuresParameters;
  LearnerParametersPtr learningParameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PREDICTOR_NUMERICAL_PARAMETERS_H_
