/*-----------------------------------------.---------------------------------.
| Filename: ProteinModel.cpp               |                                 |
| Author  : Julien Becker                  |                                 |
| Started : 08/02/2012 15:29               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinModel.h"
#include "../Data/ProteinFunctions.h"

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

FunctionPtr ProteinModel::createPerception(ExecutionContext& context) const
{
  return lbcppMemberCompositeFunction(ProteinModel, buildPerception);    
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
