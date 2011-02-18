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

class GraphBasedFunction : public FrameBasedFunction
{
public:
  virtual bool initializeFunctionGraph(const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName) = 0;

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    this->context = &context;
    frameClass = new FrameClass(T("toto"));
    if (!initializeFunctionGraph(inputVariables, outputName, outputShortName))
      return TypePtr();
    return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }

protected:
  ExecutionContext* context;

  size_t copyInput(const VariableSignaturePtr& signature)
    {return addInSelection(frameClass->addMemberVariable(*context, signature));}

  size_t addInput(TypePtr type, const String& name = String::empty)
  {
    String n = name.isEmpty() ? T("input") + String((int)frameClass->getNumMemberVariables() + 1) : name;
    return addInSelection(frameClass->addMemberVariable(*context, type, n));
  }

  size_t addInSelection(size_t index)
  {
    currentSelection.push_back(index);
    return index;
  }

  size_t addConstant(const Variable& value, const String& name = String::empty)
    {jassert(false); return 0;}

  size_t addFunction(const FunctionPtr& function, size_t input, const String& outputName = String::empty, const String& outputShortName = String::empty)
    {return addInSelection(frameClass->addMemberOperator(*context, function, input, outputName, outputShortName));}

  size_t addFunction(const FunctionPtr& function, size_t input1, size_t input2, const String& outputName = String::empty, const String& outputShortName = String::empty)
    {return addInSelection(frameClass->addMemberOperator(*context, function, input1, input2, outputName, outputShortName));}

  size_t addFunction(const FunctionPtr& function, std::vector<size_t>& inputs, const String& outputName = String::empty, const String& outputShortName = String::empty)
    {return addInSelection(frameClass->addMemberOperator(*context, function, inputs, outputName, outputShortName));}

  std::vector<size_t> currentSelection;

  void startSelection()
    {currentSelection.clear();}

  const std::vector<size_t>& finishSelection()
    {return currentSelection;}

  size_t finishSelectionWithFunction(const FunctionPtr& function)
  {
    size_t res = addFunction(function, currentSelection);
    currentSelection.clear();
    return res;
  }
};

// object, position -> element
class GetElementInVariableFunction : public GraphBasedFunction
{
public:
  GetElementInVariableFunction(const String& variableName)
    : variableName(variableName) {}
 
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 1 ? positiveIntegerType : (TypePtr)objectClass;}

  virtual bool initializeFunctionGraph(const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    copyInput(inputVariables[0]);
    addInput(positiveIntegerType);
    addFunction(getVariableFunction(variableName), 0);
    addFunction(getElementFunction(), 2, 1);
    return true;
  }

protected:
  String variableName;
};

FunctionPtr getElementInVariableFunction(const String& variableName)
  {return new GetElementInVariableFunction(variableName);}

// distribution[enumeration] -> features
class EnumerationDistributionFeaturesFunction : public GraphBasedFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return enumerationDistributionClass();}

  virtual bool initializeFunctionGraph(const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    copyInput(inputVariables[0]);
    addFunction(distributionEntropyFunction(), 0);
    
    startSelection();

      addFunction(enumerationDistributionFeatureGenerator(), 0, T("p"));
      addFunction(defaultPositiveDoubleFeatureGenerator(10, -1.0, 4.0), 1, T("e"));

    finishSelectionWithFunction(concatenateFeatureGenerator(false));
    return true;
  }
};

// position, protein -> features
class ProteinPrimaryResidueFeaturesFunction : public GraphBasedFunction
{
public:
  virtual bool initializeFunctionGraph(const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    addInput(positiveIntegerType, T("position"));
    addInput(proteinClass, T("protein"));
    size_t aminoAcid = addFunction(getElementInVariableFunction(T("primaryStructure")), 1, 0);
    size_t pssmRow = addFunction(getElementInVariableFunction(T("positionSpecificScoringMatrix")), 1, 0);
    //size_t ss3 = addFunction(getElementInVariableFunction(T("secondaryStructure")), 1, 0);
    //size_t ss8 = addFunction(getElementInVariableFunction(T("dsspSecondaryStructure")), 1, 0);

    // feature generators
    startSelection();
    
      addFunction(enumerationFeatureGenerator(), aminoAcid, T("aa"));
      addFunction(new EnumerationDistributionFeaturesFunction(), pssmRow, T("pssm"));
      //addFunction(new EnumerationDistributionFeaturesFunction(), ss3, T("ss3"));
      //addFunction(new EnumerationDistributionFeaturesFunction(), ss8, T("ss8"));
  
    finishSelectionWithFunction(concatenateFeatureGenerator(false));
    return true;
  }
};

// protein -> vector[features]
class ProteinPrimaryResidueFeaturesVectorFunction : public GraphBasedFunction
{
public:
  virtual bool initializeFunctionGraph(const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    addInput(proteinClass, T("protein"));
    addFunction(proteinLengthFunction(), 0);
    addFunction(createVectorFunction(new ProteinPrimaryResidueFeaturesFunction()), 1, 0);
    return true;
  }
};

// position, vector[features], accumulator[features], features (mean)
class ProteinResidueFeaturesFunction : public GraphBasedFunction
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 4;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? positiveIntegerType : anyType;}

  virtual bool initializeFunctionGraph(const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    size_t position = addInput(positiveIntegerType);
    size_t primaryResidueFeatures = copyInput(inputVariables[1]);
    size_t primaryResidueFeaturesAcc = copyInput(inputVariables[2]);
    
    startSelection();

      copyInput(inputVariables[3]);

      addFunction(windowFeatureGenerator(15), primaryResidueFeatures, position, T("window"));
      addFunction(accumulatorLocalMeanFunction(15), primaryResidueFeaturesAcc, position, T("mean15"));
      addFunction(accumulatorLocalMeanFunction(50), primaryResidueFeaturesAcc, position, T("mean50"));
     
    finishSelectionWithFunction(concatenateFeatureGenerator(true));
    return true;
  }
};

// protein -> vector[features]
class ProteinResidueFeaturesVectorFunction : public GraphBasedFunction
{
public:
  virtual bool initializeFunctionGraph(const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    size_t protein = addInput(proteinClass, T("protein"));

    startSelection();

      addFunction(proteinLengthFunction(), protein);
      size_t primaryFeatures = addFunction(new ProteinPrimaryResidueFeaturesVectorFunction(), protein);
      size_t primaryFeaturesAcc = addFunction(accumulateContainerFunction(), primaryFeatures);
      addFunction(accumulatorGlobalMeanFunction(), primaryFeaturesAcc, T("globalmean"));

    finishSelectionWithFunction(createVectorFunction(new ProteinResidueFeaturesFunction()));
    return true;
  }
};

namespace lbcpp
{

  FunctionPtr proteinResidueFeaturesVectorFunction()
    {return new ProteinResidueFeaturesVectorFunction();}

};

//////////////////////////////////////////
//////////////////////////////////////////
//////////////////////////////////////////

FrameClassPtr ProteinFrameFactory::createProteinFrameClass(ExecutionContext& context)
{
  FrameClassPtr res = new FrameClass(T("ProteinFrame"), objectClass);
  proteinFrame = res;
  
  // inputs
  size_t lengthIndex = res->addMemberVariable(context, positiveIntegerType, T("length"));
  res->addMemberVariable(context, vectorClass(aminoAcidTypeEnumeration), T("primaryStructure"), T("AA"));
  res->addMemberVariable(context, vectorClass(enumerationDistributionClass(aminoAcidTypeEnumeration)), T("positionSpecificScoringMatrix"), T("PSSM"));
  res->addMemberVariable(context, vectorClass(enumerationDistributionClass(secondaryStructureElementEnumeration)), T("secondaryStructure"), T("SS3"));

  // primary residue features
  FrameClassPtr primaryResidueFrameClass = createPrimaryResidueFrameClass(context, res);
  if (!primaryResidueFrameClass)
    return FrameClassPtr();
  size_t contextFreeResidueFeatures = res->addMemberOperator(context, createVectorFunction(new FrameBasedFunction(primaryResidueFrameClass)), lengthIndex, (size_t)-1, T("primaryResidueFeatures"));
  res->addMemberOperator(context, accumulateContainerFunction(), contextFreeResidueFeatures, T("primaryResidueFeaturesAcc"));
  
  // global features
  FrameClassPtr proteinGlobalFrameClass = createProteinGlobalFrameClass(context, res);
  if (!proteinGlobalFrameClass)
    return FrameClassPtr();
  res->addMemberOperator(context, new FrameBasedFunction(proteinGlobalFrameClass), (size_t)-1, T("globalFeatures"));

  // residue features
  FrameClassPtr residueFrameClass = createResidueFrameClass(context, res);
  if (!residueFrameClass)
    return FrameClassPtr();
  res->addMemberOperator(context, createVectorFunction(new FrameBasedFunction(residueFrameClass)), (size_t)-1, lengthIndex, T("residueFeatures"));

  return res->initialize(context) ? res : FrameClassPtr();
}

FrameClassPtr ProteinFrameFactory::createPrimaryResidueFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass)
{
  FrameClassPtr res = new FrameClass(T("PrimaryResidueFrame"), objectClass);
  
  // inputs
  size_t proteinFrameIndex = res->addMemberVariable(context, proteinFrameClass, T("proteinFrame"));
  size_t positionIndex = res->addMemberVariable(context, positiveIntegerType, T("position"));

  // retrieve the amino acid, the pssm row and the ss3 prediction
  size_t aaIndex = res->addMemberOperator(context, getVariableFunction(T("primaryStructure")), proteinFrameIndex);
  aaIndex = res->addMemberOperator(context, getElementFunction(), aaIndex, positionIndex, T("aa"));

  size_t pssmIndex = res->addMemberOperator(context, getVariableFunction(T("positionSpecificScoringMatrix")), proteinFrameIndex);
  pssmIndex = res->addMemberOperator(context, getElementFunction(), pssmIndex, positionIndex, T("pssm"));

  size_t ss3Index = res->addMemberOperator(context, getVariableFunction(T("secondaryStructure")), proteinFrameIndex);
  ss3Index = res->addMemberOperator(context, getElementFunction(), ss3Index, positionIndex, T("ss3"));

  // feature generators
  std::vector<size_t> featureIndices;

  featureIndices.push_back(res->addMemberOperator(context, enumerationFeatureGenerator(), aaIndex));
  featureIndices.push_back(res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), pssmIndex));
  size_t pssmEntropyIndex = res->addMemberOperator(context, distributionEntropyFunction(), pssmIndex);
  featureIndices.push_back(res->addMemberOperator(context, defaultPositiveDoubleFeatureGenerator(10, -1.0, 4.0), pssmEntropyIndex));
  featureIndices.push_back(res->addMemberOperator(context, enumerationDistributionFeatureGenerator(), ss3Index));
  size_t ss3EntropyIndex = res->addMemberOperator(context, distributionEntropyFunction(), ss3Index);
  featureIndices.push_back(res->addMemberOperator(context, defaultPositiveDoubleFeatureGenerator(10, -1.0, 4.0), ss3EntropyIndex));

  // all features
  res->addMemberOperator(context, concatenateFeatureGenerator(false), featureIndices, T("primaryResidueFeatures"));

  return res->initialize(context) ? res : FrameClassPtr();
}

FrameClassPtr ProteinFrameFactory::createResidueFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass)
{
  FrameClassPtr res = new FrameClass(T("ResidueFrame"), objectClass);

  size_t proteinFrameIndex = res->addMemberVariable(context, proteinFrameClass, T("proteinFrame"));
  size_t positionIndex = res->addMemberVariable(context, positiveIntegerType, T("position"));
  
  size_t primaryResidueFeaturesIndex = res->addMemberOperator(context, getVariableFunction(T("primaryResidueFeatures")), proteinFrameIndex);
  size_t primaryResidueFeaturesAccIndex = res->addMemberOperator(context, getVariableFunction(T("primaryResidueFeaturesAcc")), proteinFrameIndex);

  std::vector<size_t> featureIndices;
  featureIndices.push_back(res->addMemberOperator(context, windowFeatureGenerator(15), primaryResidueFeaturesIndex, positionIndex, T("window")));
  featureIndices.push_back(res->addMemberOperator(context, accumulatorLocalMeanFunction(10), primaryResidueFeaturesAccIndex, positionIndex, T("localMean10")));
  featureIndices.push_back(res->addMemberOperator(context, accumulatorLocalMeanFunction(50), primaryResidueFeaturesAccIndex, positionIndex, T("localMean50")));
  featureIndices.push_back(res->addMemberOperator(context, getVariableFunction(T("globalFeatures")), proteinFrameIndex, T("global")));
  
  // all features
  res->addMemberOperator(context, concatenateFeatureGenerator(true), featureIndices, T("AllFeatures")); 

  return res->initialize(context) ? res : FrameClassPtr();
}

FrameClassPtr ProteinFrameFactory::createProteinGlobalFrameClass(ExecutionContext& context, const FrameClassPtr& proteinFrameClass)
{
  FrameClassPtr res = new FrameClass(T("ProteinGlobal"), objectClass);

  size_t proteinFrameIndex = res->addMemberVariable(context, proteinFrameClass, T("proteinFrame"));
  size_t primaryResidueFeaturesAccIndex = res->addMemberOperator(context, getVariableFunction(T("primaryResidueFeaturesAcc")), proteinFrameIndex);
  res->addMemberOperator(context, accumulatorGlobalMeanFunction(), primaryResidueFeaturesAccIndex);

  return res->initialize(context) ? res : FrameClassPtr();
}

FramePtr ProteinFrameFactory::createFrame(const ProteinPtr& protein) const
{
  FramePtr res(new Frame(proteinFrame));
  res->setVariable(0, protein->getLength());
  res->setVariable(1, protein->getPrimaryStructure());
  res->setVariable(2, protein->getPositionSpecificScoringMatrix());
  
  /*ContainerPtr inputSecondaryStructure = protein->getSecondaryStructure();
  if (inputSecondaryStructure)
  {
    size_t n = inputSecondaryStructure->getNumElements();
    VectorPtr secondaryStructure = vector(enumerationDistributionClass(secondaryStructureElementEnumeration), n);
    for (size_t i = 0; i < n; ++i)
    {
      EnumerationDistributionPtr distribution = new EnumerationDistribution(secondaryStructureElementEnumeration);
      distribution->setProbability((size_t)inputSecondaryStructure->getElement(i).getInteger(), 1.0);
      secondaryStructure->setElement(i, distribution);
    }
    res->setVariable(3, secondaryStructure);
  }*/
  return res;
}
