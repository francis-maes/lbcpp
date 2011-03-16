/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictor.cpp           | Protein Predictor               |
| Author  : Francis Maes                   |                                 |
| Started : 19/02/2011 03:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ProteinPredictor.h"
using namespace lbcpp;

ProteinPredictor::ProteinPredictor(ProteinPredictorParametersPtr parameters = ProteinPredictorParametersPtr())
  : parameters(parameters),
    activeResiduePerception(false),
    activeResiduePairPerception(false),
    activeDisulfideResiduePairPerception(false)
{
}

void ProteinPredictor::addTarget(ProteinTarget target)
{
  FunctionPtr targetPredictor = parameters->createTargetPredictor(target);
  jassert(targetPredictor);
  targetPredictors.push_back(std::make_pair(target, targetPredictor));
  switch (target) {
    case ss3Target:
    case ss8Target:
    case stalTarget:
    case saTarget:
    case sa20Target:
    case drTarget:
      activeResiduePerception = true;
      break;
    case cma8Target:
    case cmb8Target:
    case dmaTarget:
    case dmbTarget:
      activeResiduePairPerception = true;
      break;
    case dsbTarget:
      activeDisulfideResiduePairPerception = true;
      break;
    default:
      jassertfalse;
      break;
  }
}

void ProteinPredictor::buildFunction(CompositeFunctionBuilder& builder)
{
  size_t input = builder.addInput(proteinClass, T("input"));
  size_t supervision = builder.addInput(proteinClass, T("supervision"));

  size_t residuePerception = activeResiduePerception ? builder.addFunction(parameters->createResidueVectorPerception(), input) : (size_t)-1;
  size_t residuePairPerception = activeResiduePairPerception ? builder.addFunction(parameters->createResiduePairVectorPerception(), input) : (size_t)-1;
  size_t disulfideResiduePairPerception = activeDisulfideResiduePairPerception ? 0.0 : (size_t)-1; // FIXME julien

  std::vector<size_t> makeProteinInputs;
  makeProteinInputs.push_back(input);
  for (size_t i = 0; i < targetPredictors.size(); ++i)
  {
    ProteinTarget target = targetPredictors[i].first;
    String targetName = proteinClass->getMemberVariableName(target);
    FunctionPtr targetPredictor = targetPredictors[i].second;

    size_t targetPredictorInput = 0;
    TypePtr elementsType = Container::getTemplateParameter(proteinClass->getMemberVariableType(target));
    jassert(elementsType);
    if (elementsType->inheritsFrom(doubleVectorClass(enumValueType, probabilityType)) || // label sequences
        elementsType->inheritsFrom(probabilityType))                                     // probability sequences
      targetPredictorInput = residuePerception;                                          // -> residue perceptions
    else if (elementsType->inheritsFrom(symmetricMatrixClass(probabilityType)))          // contact maps
      targetPredictorInput = (target == dsbTarget) ? disulfideResiduePairPerception : residuePairPerception; // -> residue pair perception                                     
    else
      jassert(false);

    size_t targetSupervision = builder.addFunction(new GetProteinTargetFunction(target), supervision, targetName + T("Supervision"));
    makeProteinInputs.push_back(builder.addConstant((int)target));
    makeProteinInputs.push_back(builder.addFunction(targetPredictor, targetPredictorInput, targetSupervision, targetName + T("Prediction")));
    targetPredictor->getOutputVariable()->setName(proteinClass->getMemberVariableName(target));
  }
  builder.addFunction(new MakeProteinFunction(), makeProteinInputs);
}
