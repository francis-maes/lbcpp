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

FunctionBuilder::FunctionBuilder(ExecutionContext& context, CompositeFunctionPtr function, const std::vector<VariableSignaturePtr>& inputVariables)
  : context(context), frameClass(new FrameClass(T("Function"))), inputVariables(inputVariables)
{
  function->setFrameClass(frameClass);
}

size_t FunctionBuilder::addInput(TypePtr type, const String& name)
{
  size_t inputNumber = frameClass->getNumMemberVariables();
  const VariableSignaturePtr& inputVariable = inputVariables[inputNumber];
  if (!context.checkInheritance(inputVariable->getType(), type))
    return invalidIndex();

  String n = name.isEmpty() ? inputVariable->getName() : name;
  return addInSelection(frameClass->addMemberVariable(context, inputVariable->getType(), n));
}

size_t FunctionBuilder::addConstant(const Variable& value, const String& name)
  {jassert(false); return 0;}

size_t FunctionBuilder::addFunction(const FunctionPtr& function, size_t input, const String& outputName, const String& outputShortName)
  {return addInSelection(frameClass->addMemberOperator(context, function, input, outputName, outputShortName));}

size_t FunctionBuilder::addFunction(const FunctionPtr& function, size_t input1, size_t input2, const String& outputName, const String& outputShortName)
  {return addInSelection(frameClass->addMemberOperator(context, function, input1, input2, outputName, outputShortName));}

size_t FunctionBuilder::addFunction(const FunctionPtr& function, std::vector<size_t>& inputs, const String& outputName, const String& outputShortName)
  {return addInSelection(frameClass->addMemberOperator(context, function, inputs, outputName, outputShortName));}

void FunctionBuilder::startSelection()
  {currentSelection.clear();}

const std::vector<size_t>& FunctionBuilder::finishSelection()
  {return currentSelection;}

size_t FunctionBuilder::finishSelectionWithFunction(const FunctionPtr& function)
{
  size_t res = addFunction(function, currentSelection);
  currentSelection.clear();
  return res;
}

size_t FunctionBuilder::addInSelection(size_t index)
{
  currentSelection.push_back(index);
  return index;
}

////////////////////////////////////////////////////

// object, position -> element
class GetElementInVariableFunction : public CompositeFunction
{
public:
  GetElementInVariableFunction(const String& variableName)
    : variableName(variableName) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual void buildFunction(FunctionBuilder& builder)
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

  virtual void buildFunction(FunctionBuilder& builder)
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

void NumericalProteinFunctionFactory::primaryResidueFeatures(FunctionBuilder& builder) const 
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

void NumericalProteinFunctionFactory::primaryResidueFeaturesVector(FunctionBuilder& builder) const
{
  builder.addInput(proteinClass, T("protein"));
  builder.addFunction(proteinLengthFunction(), 0);
  builder.addFunction(createVectorFunction(function(&NumericalProteinFunctionFactory::primaryResidueFeatures)), 1, 0);
}

void NumericalProteinFunctionFactory::residueFeatures(FunctionBuilder& builder) const
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

void NumericalProteinFunctionFactory::residueFeaturesVector(FunctionBuilder& builder) const
{
  size_t protein = builder.addInput(proteinClass, T("protein"));

  builder.startSelection();

    builder.addFunction(proteinLengthFunction(), protein);
    size_t primaryFeatures = builder.addFunction(function(&NumericalProteinFunctionFactory::primaryResidueFeaturesVector), protein);
    size_t primaryFeaturesAcc = builder.addFunction(accumulateContainerFunction(), primaryFeatures);
    builder.addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("globalmean"));

    builder.finishSelectionWithFunction(createVectorFunction(function(&NumericalProteinFunctionFactory::residueFeatures)));
}

namespace lbcpp
{

  FunctionPtr proteinResidueFeaturesVectorFunction()
  {
    NumericalProteinFunctionFactoryPtr factory = new NumericalProteinFunctionFactory();
    return factory->function(&NumericalProteinFunctionFactory::residuePerceptions);
  }

};
