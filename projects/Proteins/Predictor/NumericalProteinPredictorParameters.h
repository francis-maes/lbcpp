/*-----------------------------------------.---------------------------------.
| Filename: NumericalProteinPredictorPar..h| Numerical Protein Predictor     |
| Author  : Francis Maes                   |  Parameters                     |
| Started : 01/03/2011 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PREDICTOR_NUMERICAL_PARAMETERS_H_
# define LBCPP_PROTEINS_PREDICTOR_NUMERICAL_PARAMETERS_H_

# include "ProteinPredictor.h"
# include "ProteinPerception.h"
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

  void addEnumerationDistributionFeatureGenerator(CompositeFunctionBuilder& builder, size_t inputIndex, const String& outputName, size_t probabilityDiscretization, size_t entropyDiscretization) const
  {
    if (probabilityDiscretization || entropyDiscretization)
      builder.addFunction(enumerationDistributionFeatureGenerator(probabilityDiscretization, entropyDiscretization), inputIndex, outputName);
  }

  void addBinaryDistributionFeatureGenerator(CompositeFunctionBuilder& builder, size_t inputIndex, const String& outputName, size_t discretization) const
  {
    if (discretization)
      builder.addFunction(defaultProbabilityFeatureGenerator(discretization), inputIndex, outputName);
  }

  /* Protein Perception */
  virtual void proteinPerception(CompositeFunctionBuilder& builder) const
  {
    builder.startSelection();

      size_t protein = builder.addInput(proteinClass, T("protein"));
      size_t length = builder.addFunction(proteinLengthFunction(), protein);
      size_t primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, primaryResidueFeatures)), length, protein);
      size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("globalmean"));

    builder.finishSelectionWithFunction(new CreateProteinPerceptionFunction());
  }
  
  void primaryResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));

    size_t aminoAcid = builder.addFunction(getElementInVariableFunction(T("primaryStructure")), protein, position);
    size_t pssmRow = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position);
    size_t ss3 = builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), protein, position);
    size_t ss8 = builder.addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), protein, position);
    size_t stal = builder.addFunction(getElementInVariableFunction(T("structuralAlphabetSequence")), protein, position);
    size_t sa20 = builder.addFunction(getElementInVariableFunction(T("solventAccessibilityAt20p")), protein, position);
    size_t dr = builder.addFunction(getElementInVariableFunction(T("disorderRegions")), protein, position);

    // feature generators
    builder.startSelection();

      builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));
      addEnumerationDistributionFeatureGenerator(builder, pssmRow, T("pssm"), featuresParameters->pssmDiscretization, featuresParameters->pssmEntropyDiscretization);
      addEnumerationDistributionFeatureGenerator(builder, ss3, T("ss3"), featuresParameters->ss3Discretization, featuresParameters->ss3EntropyDiscretization);
      addEnumerationDistributionFeatureGenerator(builder, ss8, T("ss8"), featuresParameters->ss8Discretization, featuresParameters->ss8EntropyDiscretization);
      addEnumerationDistributionFeatureGenerator(builder, stal, T("stal"), featuresParameters->stalDiscretization, featuresParameters->stalEntropyDiscretization);
      addBinaryDistributionFeatureGenerator(builder, sa20, T("sa20"), featuresParameters->sa20Discretization);
      addBinaryDistributionFeatureGenerator(builder, dr, T("dr"), featuresParameters->drDiscretization);

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
  }
  
  /* Residue Perception */
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType));
    
    builder.startSelection();

      builder.addFunction(getVariableFunction(T("length")), proteinPerception);
      builder.addFunction(getVariableFunction(T("primaryResidueFeatures")), proteinPerception);
      builder.addFunction(getVariableFunction(T("accumulator")), proteinPerception);
      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception);

    builder.finishSelectionWithFunction(createVectorFunction(
            lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residueFeatures)), T("residueFeatureVectors"));
  }

  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t position = builder.addInput(positiveIntegerType);    
    size_t primaryResidueFeatures = builder.addInput(vectorClass(doubleVectorClass()));
    size_t primaryResidueFeaturesAcc = builder.addInput(containerClass(doubleVectorClass()));
    size_t globalFeatures = builder.addInput(doubleVectorClass());
    
    builder.startSelection();

      if (featuresParameters->residueGlobalFeatures)
        builder.addInSelection(globalFeatures);

      if (featuresParameters->residueWindowSize)
        builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->residueWindowSize), primaryResidueFeatures, position, T("window"));

      if (featuresParameters->residueLocalMeanSize)
        builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residueLocalMeanSize), primaryResidueFeaturesAcc, position, T("mean") + String((int)featuresParameters->residueLocalMeanSize));

      if (featuresParameters->residueMediumMeanSize)
        builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residueMediumMeanSize), primaryResidueFeaturesAcc, position, T("mean") + String((int)featuresParameters->residueMediumMeanSize));
     
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  /* Residue Pair Perception */
  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType));
    
    builder.startSelection();
    
      builder.addFunction(getVariableFunction(T("length")), proteinPerception);
      builder.addFunction(getVariableFunction(T("primaryResidueFeatures")), proteinPerception);
      builder.addFunction(getVariableFunction(T("accumulator")), proteinPerception);
      builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception);

    builder.finishSelectionWithFunction(createSymmetricMatrixFunction(
                  lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residuePairFeatures)), T("residueFeaturesSymmetricMatrix"));
  }

  void residuePairFeatures(CompositeFunctionBuilder& builder) const
  {
    size_t firstPosition = builder.addInput(positiveIntegerType);
    size_t secondPosition = builder.addInput(positiveIntegerType);
    size_t primaryResidueFeatures = builder.addInput(vectorClass(doubleVectorClass()));
    size_t primaryResidueFeaturesAcc = builder.addInput(containerClass(doubleVectorClass()));
    size_t globalFeatures = builder.addInput(doubleVectorClass());

    size_t aaDist = (size_t)-1;
    if (featuresParameters->aminoAcidDistanceFeature)
    {
      aaDist = builder.addFunction(new SubtractFunction(), secondPosition, firstPosition);
      aaDist = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(0, 3, 10, true), aaDist, T("aaDistance"));
    }

    size_t cartesianFirstWindow = (size_t)-1;
    size_t cartesianSecondWindow = (size_t)-1;
    if (featuresParameters->cartesianProductPrimaryWindowSize)
    {
      cartesianFirstWindow = builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->cartesianProductPrimaryWindowSize), primaryResidueFeatures, firstPosition);
      cartesianSecondWindow = builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->cartesianProductPrimaryWindowSize), primaryResidueFeatures, secondPosition);
    }
    
    builder.startSelection();

    if (featuresParameters->residuePairGlobalFeatures)
      builder.addInSelection(globalFeatures);

    if (aaDist != (size_t)-1)
      builder.addInSelection(aaDist);
    
    if (featuresParameters->useIntervalMean)
      builder.addFunction(accumulatorWindowMeanFunction(), primaryResidueFeaturesAcc, firstPosition, secondPosition, T("interval"));

    if (featuresParameters->residuePairWindowSize)
    {
      builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->residuePairWindowSize), primaryResidueFeatures, firstPosition, T("window_1_"));
      builder.addFunction(centeredContainerWindowFeatureGenerator(featuresParameters->residuePairWindowSize), primaryResidueFeatures, secondPosition, T("window_2_"));
    }
    
    if (featuresParameters->residuePairLocalMeanSize)
    {
      builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residuePairLocalMeanSize), primaryResidueFeaturesAcc, firstPosition, T("mean_1_") + String((int)featuresParameters->residueLocalMeanSize));
      builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residuePairLocalMeanSize), primaryResidueFeaturesAcc, secondPosition, T("mean_2_") + String((int)featuresParameters->residueLocalMeanSize));
    }

    if (featuresParameters->residuePairMediumMeanSize)
    {
      builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residuePairMediumMeanSize), primaryResidueFeaturesAcc, firstPosition, T("mean_1_") + String((int)featuresParameters->residueMediumMeanSize));
      builder.addFunction(accumulatorLocalMeanFunction(featuresParameters->residuePairMediumMeanSize), primaryResidueFeaturesAcc, secondPosition, T("mean_2_") + String((int)featuresParameters->residueMediumMeanSize));
    }

    if (cartesianFirstWindow != (size_t)-1 && cartesianSecondWindow != (size_t)-1)
      builder.addFunction(cartesianProductFeatureGenerator(true), cartesianFirstWindow, cartesianSecondWindow, T("cartesianProduct") + String((int)featuresParameters->cartesianProductPrimaryWindowSize));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  /* Dissulfide Residue Pair Perception */
  virtual void disulfideResiduePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType));
    
    builder.startSelection();
    
    builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
    builder.addFunction(getVariableFunction(T("primaryResidueFeatures")), proteinPerception);
    builder.addFunction(getVariableFunction(T("accumulator")), proteinPerception);
    builder.addFunction(getVariableFunction(T("globalFeatures")), proteinPerception);
    
    builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(lbcppMemberCompositeFunction(NumericalProteinPredictorParameters, residuePairFeatures)), T("residueFeaturesSymmetricMatrix"));
  }

  // Learning Machine
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    switch (target)
    {
    case drTarget:
      {
        FunctionPtr res = linearBinaryClassifier(learningParameters, true, binaryClassificationMCCScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore));
        return res;
      }

    case sa20Target:
      {
        FunctionPtr res = linearBinaryClassifier(learningParameters, true, binaryClassificationAccuracyScore);
        res->setEvaluator(rocAnalysisEvaluator(binaryClassificationAccuracyScore));
        return res;
      }

    default:
      {
        FunctionPtr res = linearLearningMachine(learningParameters);
        res->setEvaluator(defaultSupervisedEvaluator());
        return res;
      }
    };
  }

  NumericalProteinFeaturesParametersPtr featuresParameters; // TODO arnaud : accessor

protected:
  friend class NumericalProteinPredictorParametersClass;

  LearnerParametersPtr learningParameters;
};

typedef ReferenceCountedObjectPtr<NumericalProteinPredictorParameters> NumericalProteinPredictorParametersPtr;
  
extern ClassPtr numericalProteinPredictorParametersClass;  
  
}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PREDICTOR_NUMERICAL_PARAMETERS_H_
