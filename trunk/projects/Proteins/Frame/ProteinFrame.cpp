/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.cpp               | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinFrame.h"
#include "ProteinResidueFrame.h"
using namespace lbcpp;

// object, position -> element
class GetElementInVariableFunction : public CompositeFunction
{
public:
  GetElementInVariableFunction(const String& variableName)
    : variableName(variableName) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(objectClass);
    size_t position = builder.addInput(positiveIntegerType);
    size_t container = builder.addFunction(getVariableFunction(variableName), input);
    builder.addFunction(getElementFunction(), container, position);
  }

protected:
  String variableName;
};

FunctionPtr getElementInVariableFunction(const String& variableName)
  {return new GetElementInVariableFunction(variableName);}

// distribution[enumeration] -> features
class EnumerationDistributionFeaturesFunction : public CompositeFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(enumerationDistributionClass());
    size_t entropy = builder.addFunction(distributionEntropyFunction(), input);

    builder.startSelection();

      builder.addFunction(enumerationDistributionFeatureGenerator(), input, T("p"));
      builder.addFunction(defaultPositiveDoubleFeatureGenerator(10, -1.0, 4.0), entropy, T("e"));

    builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
  }
};

//////////////////////////////////////////////

void NumericalProteinPredictorParameters::primaryResidueFeatures(CompositeFunctionBuilder& builder) const 
{
  builder.addInput(positiveIntegerType, T("position"));
  builder.addInput(proteinClass, T("protein"));
  size_t aminoAcid = builder.addFunction(getElementInVariableFunction(T("primaryStructure")), 1, 0);
  size_t pssmRow = builder.addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), 1, 0);
  //size_t ss3 = builder.addFunction(getElementInVariableFunction(T("secondaryStructure")), 1, 0);
  //size_t ss8 = builder.addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), 1, 0);

  // feature generators
  builder.startSelection();
  
    builder.addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));
    builder.addFunction(new EnumerationDistributionFeaturesFunction(), pssmRow, T("pssm"));
    //addFunction(new EnumerationDistributionFeaturesFunction(), ss3, T("ss3"));
    //addFunction(new EnumerationDistributionFeaturesFunction(), ss8, T("ss8"));

  builder.finishSelectionWithFunction(concatenateFeatureGenerator(false));
}

void NumericalProteinPredictorParameters::primaryResidueFeaturesVector(CompositeFunctionBuilder& builder) const
{
  builder.addInput(proteinClass, T("protein"));
  builder.addFunction(proteinLengthFunction(), 0);
  builder.addFunction(createVectorFunction(function(&NumericalProteinPredictorParameters::primaryResidueFeatures)), 1, 0);
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

    builder.finishSelectionWithFunction(createVectorFunction(function(&NumericalProteinPredictorParameters::residueFeatures)));
}

namespace lbcpp
{

  FunctionPtr proteinResidueFeaturesVectorFunction()
  {
    NumericalProteinPredictorParametersPtr factory = new NumericalProteinPredictorParameters();
    return factory->function(&NumericalProteinPredictorParameters::residueVectorPerception);
  }

};
