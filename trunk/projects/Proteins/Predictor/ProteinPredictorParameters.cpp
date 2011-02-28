/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictorParameters.cpp | Protein Predictor Parameters    |
| Author  : Francis Maes                   |                                 |
| Started : 02/02/2011 14:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinPredictorParameters.h"
#include <lbcpp/Distribution/DiscreteDistribution.h>
#include <lbcpp/FeatureGenerator/FeatureGenerator.h>
#include <lbcpp/Learning/Numerical.h>
using namespace lbcpp;

// DV[enumeration, probabilityType] -> features
class EnumerationDistributionFeaturesFunction : public CompositeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(doubleVectorClass(enumValueType, probabilityType), T("p"));
    size_t entropy = builder.addFunction(doubleVectorEntropyFunction(), input);
    size_t entropyFeatures = builder.addFunction(defaultPositiveDoubleFeatureGenerator(10, -1.0, 4.0), entropy, T("e"));
    builder.addFunction(concatenateFeatureGenerator(false), input, entropyFeatures);
  }
};

/*
** NumericalProteinPredictorParameters
*/
void NumericalProteinPredictorParameters::primaryResidueFeatures(CompositeFunctionBuilder& builder) const 
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
    builder.addFunction(new EnumerationDistributionFeaturesFunction(), pssmRow, T("pssm"));
    builder.addFunction(new EnumerationDistributionFeaturesFunction(), ss3, T("ss3"));
    builder.addFunction(new EnumerationDistributionFeaturesFunction(), ss8, T("ss8"));
    builder.addFunction(new EnumerationDistributionFeaturesFunction(), stal, T("stal"));
    builder.addFunction(defaultProbabilityFeatureGenerator(10), sa20, T("sa20"));
    builder.addFunction(defaultProbabilityFeatureGenerator(10), dr, T("dr"));

  builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
}

void NumericalProteinPredictorParameters::primaryResidueFeaturesVector(CompositeFunctionBuilder& builder) const
{
  size_t input = builder.addInput(proteinClass, T("protein"));
  size_t length = builder.addFunction(proteinLengthFunction(), 0);
  builder.addFunction(createVectorFunction(function(&NumericalProteinPredictorParameters::primaryResidueFeatures)), length, input);
}

void NumericalProteinPredictorParameters::residueFeatures(CompositeFunctionBuilder& builder) const
{
  size_t position = builder.addInput(positiveIntegerType);
  size_t primaryResidueFeatures = builder.addInput(vectorClass(doubleVectorClass()));
  size_t primaryResidueFeaturesAcc = builder.addInput(containerClass());
  
  builder.startSelection();

    builder.addInput(doubleVectorClass());

    builder.addFunction(windowFeatureGenerator(15), primaryResidueFeatures, position, T("window"));
    builder.addFunction(accumulatorLocalMeanFunction(15), primaryResidueFeaturesAcc, position, T("mean15"));
    builder.addFunction(accumulatorLocalMeanFunction(50), primaryResidueFeaturesAcc, position, T("mean50"));
   
  builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));

}

void NumericalProteinPredictorParameters::residueFeaturesVector(CompositeFunctionBuilder& builder) const
{
  size_t protein = builder.addInput(proteinClass, T("protein"));

  builder.startSelection();

    builder.addFunction(proteinLengthFunction(), protein);
    size_t primaryFeatures = builder.addFunction(function(&NumericalProteinPredictorParameters::primaryResidueFeaturesVector), protein);
    size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
    builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("globalmean"));

  builder.finishSelectionWithFunction(createVectorFunction(function(&NumericalProteinPredictorParameters::residueFeatures)), T("residueFeatureVectors"));
}

FunctionPtr NumericalProteinPredictorParameters::learningMachine(ProteinTarget target) const
  {return linearLearningMachine(new StochasticGDParameters());}
