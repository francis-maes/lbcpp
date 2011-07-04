/*-----------------------------------------.---------------------------------.
| Filename: Lin09PredictorParameters.h     | Numerical Predictor             |
| Author  : Julien Becker                  |  Parameters from Lin et al. 09  |
| Started : 11/06/2011 10:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LIN09_H_
# define LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LIN09_H_

# include "ProteinPredictor.h"
# include "ProteinPerception.h"
# include "ProteinPredictorParameters.h"
# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

class Lin09PredictorParameters : public ProteinPredictorParameters
{
public:
  Lin09PredictorParameters() : C(7.4), kernelGamma(-4.6), useLibSVM(true), learningRate(1.0), numIterations(150) {}
  /*
  ************************ Protein Perception ************************
  */
  virtual void proteinPerception(CompositeFunctionBuilder& builder) const
  {
    builder.startSelection();

      size_t protein = builder.addInput(proteinClass, T("protein"));
      size_t length = builder.addFunction(new ProteinLengthFunction(), protein);
      builder.addFunction(new NumCysteinsFunction(), protein);
      size_t primaryFeatures = builder.addFunction(createVectorFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, primaryResidueFeatures)), length, protein);
      size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
      //builder.addFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, globalFeatures), protein, primaryFeaturesAcc, T("globalFeatures"));
      builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 0, 0.0));

    builder.finishSelectionWithFunction(new CreateProteinPerceptionFunction());
  }

  /*
  ************************ Property Perception ************************
  */
  virtual void propertyPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

  void primaryResidueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t protein = builder.addInput(proteinClass, T("protein"));

    /* Precompute */
    size_t pssmRow = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), protein, position, T("pssm"));

    /* feature generators */
    builder.startSelection();
      builder.addInSelection(pssmRow);
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
  }

  /*
  ************************ Residue Perception ************************
  */
  virtual void residueVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  void residueFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t position = builder.addInput(positiveIntegerType, T("position"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    /* Data */
    size_t primaryResidueFeatures = builder.addFunction(getVariableFunction(T("primaryResidueFeatures")), proteinPerception);

    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception);

    /* Output */
    builder.startSelection();
      builder.addFunction(centeredContainerWindowFeatureGenerator(23), primaryResidueFeatures, position, T("w"));
      builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), protein, position, T("ss3"));
    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }
  
  /*
  ************************ Cystein Residue Perception ************************
  */
  virtual void cysteinResiudeVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  /*
  ************************ Residue Pair Perception ************************
  */
  virtual void residuePairVectorPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

  /*
  ************************ Cystein Residue Pair Perception ************************
  */
  virtual void cysteinResiudePairVectorPerception(CompositeFunctionBuilder& builder) const
  {
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));
    
    builder.startSelection();

      builder.addFunction(getVariableFunction(T("protein")), proteinPerception);
      builder.addInSelection(proteinPerception);

    builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(
            lbcppMemberCompositeFunction(Lin09PredictorParameters, cysteinResiduePairVectorFeatures))
                                        , T("cysteinResiduePairFeatures"));
  }
  
  void cysteinResiduePairVectorFeatures(CompositeFunctionBuilder& builder) const
  {
    /* Inputs */
    size_t firstPosition = builder.addInput(positiveIntegerType, T("firstPosition"));
    size_t secondPosition = builder.addInput(positiveIntegerType, T("secondPosition"));
    size_t proteinPerception = builder.addInput(numericalProteinPrimaryFeaturesClass(enumValueType, enumValueType));

    size_t protein = builder.addFunction(getVariableFunction(T("protein")), proteinPerception, T("protein"));
    size_t firstIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, firstPosition);
    size_t secondIndex = builder.addFunction(new GetCysteinIndexFromProteinIndex(), protein, secondPosition);
    
    /* Output */
    builder.startSelection();

      builder.addFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, residueFeatures), firstPosition, proteinPerception, T("rf[first]"));
      builder.addFunction(lbcppMemberCompositeFunction(Lin09PredictorParameters, residueFeatures), secondPosition, proteinPerception, T("rf[second]"));

      builder.addFunction(new NormalizedCysteinPositionDifference(), protein, firstPosition, secondPosition, T("NCPD"));
      builder.addFunction(new NormalizedCysteinIndexDifference(), protein, firstIndex, secondIndex, T("NCID"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
  }

  void cysteinResiduePairFeatures(CompositeFunctionBuilder& builder) const
    {jassertfalse;}
  
  /*
   ************************ Multi Task Features ************************
   */

  // Learning Machine
  virtual FunctionPtr createTargetPredictor(ProteinTarget target) const
  {
    if (target == dsbTarget)
      return new ConnectivityPatternClassifier(new StochasticGDParameters(constantIterationFunction(learningRate), StoppingCriterionPtr(), numIterations));
    jassertfalse;
    return FunctionPtr();
  }

  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    switch (target)
    {
    case dsbTarget:
      {
        if (useLibSVM)
          return libSVMBinaryClassifier(pow(2.0, C), rbfKernel, 0, pow(2.0, kernelGamma), 0.0);
      }
    default:
      {
        jassertfalse;
        return FunctionPtr();
      }
    };
  }

protected:
  friend class Lin09PredictorParametersClass;
  
  double C;
  double kernelGamma;
  bool useLibSVM;
  double learningRate;
  size_t numIterations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_PREDICTOR_PARAMETERS_LIN09_H_
