/*-----------------------------------------.---------------------------------.
| Filename: ProteinModel.cpp               |                                 |
| Author  : Julien Becker                  |                                 |
| Started : 08/02/2012 15:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinModel.h"
#include "ProteinMap.h"
#include "../Data/ProteinFunctions.h"
#include "../Predictor/ProteinPerception.h"

using namespace lbcpp;


void Model::buildFunction(CompositeFunctionBuilder& builder)
{
  size_t input = builder.addInput(anyType);
  size_t supervision = builder.addInput(anyType);

  size_t perception = builder.addFunction(createPerception(builder.getContext()), input, T("Perception"));
  builder.addFunction(createPredictor(builder.getContext()), perception, supervision, T("Prediction"));
}  

void ProteinModel::buildFunction(CompositeFunctionBuilder& builder)
{
  const String targetName = proteinClass->getMemberVariableName(target);

  size_t inputProtein = builder.addInput(proteinClass);
  size_t supervisionProtein = builder.addInput(proteinClass);

  size_t perception = builder.addFunction(createPerception(builder.getContext()), inputProtein, targetName + T("Perception"));
  size_t supervision = builder.addFunction(new GetProteinTargetFunction(target), supervisionProtein, targetName + T("Supervision"));

  std::vector<size_t> makeProteinInputs;
  makeProteinInputs.push_back(inputProtein);
  makeProteinInputs.push_back(builder.addConstant((int)target));
  makeProteinInputs.push_back(builder.addFunction(createPredictor(builder.getContext()), perception, supervision, targetName + T("Prediction")));

  builder.addFunction(new MakeProteinFunction(), makeProteinInputs);
}

FunctionPtr ProteinModel::createPredictor(ExecutionContext& context) const
{
  switch (target)
  {
    case ss3Target:
    case ss8Target:
    case stalTarget:
      // MultiClass Sequence Labeling
      // Vector[Input], Vector[DoubleVector[Probability]] -> Vector[DoubleVector[Probability]]
      return mapNContainerFunction(createMachineLearning(context));
    case sa20Target:
    case drTarget:
    case cbsTarget:
      // Binary Sequence Labeling
      // Vector[Input], Vector[Probability/Boolean] -> Vector[Probability]
      return mapNContainerFunction(createMachineLearning(context));
    case cbpTarget:
      // Binary Classification
      // Input, Probability/Boolean -> Probability
      return createMachineLearning(context);
    case dsbTarget:
      // Symmetric Matrix Binary Classification
      // SymmetricMatrix[Input], SymmetricMatrix[Probability] -> SymmetricMatrix[Probability]
      return mapNSymmetricMatrixFunction(createMachineLearning(context), 1);
    default:
      const String targetName = proteinClass->getMemberVariableName(target);
      context.errorCallback(T("ProteinModel::createMachineLearning"),
                            T("No implementation found for ") + targetName);
  }

  return FunctionPtr();
}

FunctionPtr ProteinModel::createPerception(ExecutionContext& context) const
{
  return lbcppMemberCompositeFunction(ProteinModel, buildPerception);
}

void ProteinModel::buildPerception(CompositeFunctionBuilder& builder) const
{
  size_t protein = builder.addInput(proteinClass);
  size_t proteinMap = builder.addFunction(new CreateProteinMap(), protein, T("ProteinMap"));
 
  switch (target)
  {
    case cbpTarget:
      // Protein
      builder.addFunction(lbcppMemberCompositeFunction(ProteinModel, buildGlobalPerception), proteinMap, T("globalPerception"));
      return;
    case ss3Target:
    case ss8Target:
    case stalTarget:
    case sa20Target:
    case drTarget:
      // Sequence
      builder.startSelection();
      {
        builder.addFunction(getVariableFunction(T("length")), proteinMap);
        builder.addInSelection(proteinMap);
      }
      builder.finishSelectionWithFunction(createVectorFunction(lbcppMemberCompositeUnlearnableFunction(ProteinModel, buildResiduePerception)), T("residuePerception"));
      return;
    case cbsTarget:
      // Cysteine Sequence
      builder.startSelection();
      {
        builder.addFunction(getVariableFunction(T("protein")), proteinMap);
        builder.addInSelection(proteinMap);
      }
      builder.finishSelectionWithFunction(new CreateCysteinBondingStateVectorFunction(lbcppMemberCompositeUnlearnableFunction(ProteinModel, buildCysteineResiduePerception)), T("cysteinePerception"));
      return;
    case dsbTarget:
      // Cysteine Pair
      builder.startSelection();
      {
        builder.addFunction(getVariableFunction(T("protein")), proteinMap);
        builder.addInSelection(proteinMap);
      }
      builder.finishSelectionWithFunction(new CreateDisulfideSymmetricMatrixFunction(lbcppMemberCompositeFunction(ProteinModel, buildCysteineResiduePairPerception), 0.f), T("cysteinePairPerception"));
      return;
    default:
      const String targetName = proteinClass->getMemberVariableName(target);
      builder.getContext().errorCallback(T("ProteinModel::buildPerception"),
                                         T("No implementation found for ") + targetName);
  }
}

void ProteinModel::buildGlobalPerception(CompositeFunctionBuilder& builder) const
{
  size_t proteinMap = builder.addInput(proteinMapClass);
  
  builder.startSelection();
  {
    builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 1, 1.0), T("bias"));
    builder.addFunction(lbcppMemberCompositeUnlearnableFunction(ProteinModel, globalFeatures), proteinMap, T("global"));
  }
  builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
}

void ProteinModel::buildResiduePerception(CompositeFunctionBuilder& builder) const
{
  size_t position = builder.addInput(positiveIntegerType);
  size_t proteinMap = builder.addInput(proteinMapClass);
  
  // Global Features
  size_t globalFeatures = builder.addFunction(lbcppMemberCompositeUnlearnableFunction(ProteinModel, globalFeatures), proteinMap, T("global"));
  
  // Residue Features
  builder.startSelection();
  {
    builder.addInSelection(position);
    builder.addInSelection(proteinMap);
  }
  size_t residueFeatures = builder.finishSelectionWithFunction(lbcppMemberCompositeUnlearnableFunction(ProteinModel, residueFeatures), T("residue"));
  
  // Concatenate features
  builder.startSelection();
  {
    builder.addConstant(new DenseDoubleVector(singletonEnumeration, doubleType, 1, 1.0), T("bias"));
    builder.addInSelection(globalFeatures);
    builder.addInSelection(residueFeatures);
  }
  builder.finishSelectionWithFunction(concatenateFeatureGenerator(true));
}

void ProteinParallelModel::buildFunction(CompositeFunctionBuilder& builder)
{
  size_t inputProtein = builder.addInput(proteinClass);
  size_t supervisionProtein = builder.addInput(proteinClass);

  std::vector<size_t> makeProteinInputs;
  makeProteinInputs.push_back(inputProtein);

  for (size_t i = 0; i < models.size(); ++i)
  {
    const String targetName = proteinClass->getMemberVariableName(models[i]->getProteinTarget());

    size_t perception = builder.addFunction(models[i]->createPerception(builder.getContext()), inputProtein, targetName + T("Perception"));
    size_t supervision = builder.addFunction(new GetProteinTargetFunction(models[i]->getProteinTarget()), supervisionProtein, targetName + T("Supervision"));

    makeProteinInputs.push_back(builder.addConstant((int)models[i]->getProteinTarget()));
    makeProteinInputs.push_back(builder.addFunction(models[i]->createPredictor(builder.getContext()), perception, supervision, targetName + T("Prediction")));
  }

  builder.addFunction(new MakeProteinFunction(), makeProteinInputs);
}

void ProteinSequentialModel::buildFunction(CompositeFunctionBuilder& builder)
{
  size_t inputProtein = builder.addInput(proteinClass);
  size_t supervisionProtein = builder.addInput(proteinClass);
  
  size_t lastModel = inputProtein;
  for (size_t i = 0; i < models.size(); ++i)
    lastModel = builder.addFunction(models[i], lastModel, supervisionProtein, T("Stage ") + String((int)i + 1));
}
