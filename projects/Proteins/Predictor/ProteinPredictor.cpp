/*-----------------------------------------.---------------------------------.
| Filename: ProteinPredictor.cpp           | Protein Predictor               |
| Author  : Francis Maes                   |                                 |
| Started : 19/02/2011 03:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ProteinPredictor.h"
#include "ProteinPerception.h"

using namespace lbcpp;

ProteinPredictor::ProteinPredictor(ProteinPredictorParametersPtr parameters = ProteinPredictorParametersPtr())
  : parameters(parameters),
    activeGlobalPerception(false),
    activeResiduePerception(false),
    activeResiduePairPerception(false),
    activeDisulfideResiduePairPerception(false),
    activeDisulfideSymmetricResiduePairPerception(false),
    activeCysteinResiduePerception(false)
{
}

void ProteinPredictor::addTarget(ProteinTarget target)
{
  FunctionPtr targetPredictor = parameters->createTargetPredictor(target);
  jassert(targetPredictor);
  targetPredictors.push_back(std::make_pair(target, targetPredictor));
  switch (typeOfProteinPerception(target))
  {
    case globalType:
      activeGlobalPerception = true;
      break;
    case residueType:
      activeResiduePerception = true;
      break;
    case residuePairType:
      activeResiduePairPerception = true;
      break;
    case disulfideBondType:
      activeDisulfideResiduePairPerception = true;
      break;
    case disulfideSymmetricBondType:
      activeDisulfideSymmetricResiduePairPerception = true;
      break;
    case cysteinBondingStateType:
      activeCysteinResiduePerception = true;
      break;
    default:
      jassertfalse;
      break;
  }
}

void ProteinPredictor::buildFunction(CompositeFunctionBuilder& builder)
{
  /* Inputs */
  size_t input = builder.addInput(proteinClass, T("input"));
  size_t supervision = builder.addInput(proteinClass, T("supervision"));

  /* Perceptions */
  size_t proteinPerception = builder.addFunction(parameters->createProteinPerception(), input);
  size_t propertyPerception = activeGlobalPerception ? builder.addFunction(parameters->createGlobalPerception(), proteinPerception) : (size_t)-1;
  size_t residuePerception = activeResiduePerception ? builder.addFunction(parameters->createResidueVectorPerception(), proteinPerception) : (size_t)-1;
  size_t residuePairPerception = activeResiduePairPerception ? builder.addFunction(parameters->createResiduePairVectorPerception(), proteinPerception) : (size_t)-1;
  size_t disulfideSymmetricResiduePerception = activeDisulfideSymmetricResiduePairPerception ? builder.addFunction(parameters->createDisulfideSymmetricResiduePairVectorPerception(), proteinPerception) : (size_t)-1;
  size_t disulfideResiduePerception = activeDisulfideResiduePairPerception ? builder.addFunction(parameters->createDisulfideResiduePairVectorPerception(), proteinPerception) : (size_t)-1;
  size_t cysteinBondingStatePerception = activeCysteinResiduePerception ? builder.addFunction(parameters->createCysteinBondingStateVectorPerception(), proteinPerception) : (size_t)-1;

  /* Dynamic Types - Backup */
  if (residuePerception != (size_t)-1)
    residuePerceptionType = getStateClass()->getMemberVariableType(residuePerception);
  if (residuePairPerception != (size_t)-1)
    residuePairPerceptionType = getStateClass()->getMemberVariableType(residuePairPerception);

  /* Associate a perception for each task */
  std::vector<size_t> makeProteinInputs;
  makeProteinInputs.push_back(input);
  for (size_t i = 0; i < targetPredictors.size(); ++i)
  {
    ProteinTarget target = targetPredictors[i].first;
    ProteinPerceptionType targetPerceptionType = typeOfProteinPerception(target);

    size_t targetPredictorInput = 0;
    switch (targetPerceptionType)
    {
      case globalType:
        targetPredictorInput = propertyPerception;
        break;
      case residueType:
        targetPredictorInput = residuePerception;
        break;
      case residuePairType:
        targetPredictorInput = residuePairPerception;
        break;
      case disulfideSymmetricBondType:
        targetPredictorInput = disulfideSymmetricResiduePerception;
        break;
      case disulfideBondType:
        targetPredictorInput = disulfideResiduePerception;
        break;
      case cysteinBondingStateType:
        targetPredictorInput = cysteinBondingStatePerception;
        break;
      default:
        jassertfalse;
    }

    /* Predict */
    String targetName = proteinClass->getMemberVariableName(target);
    FunctionPtr targetPredictor = targetPredictors[i].second;

    size_t targetSupervision = builder.addFunction(new GetProteinTargetFunction(target), supervision, targetName + T("Supervision"));
    makeProteinInputs.push_back(builder.addConstant((int)target));
    makeProteinInputs.push_back(builder.addFunction(targetPredictor, targetPredictorInput, targetSupervision, targetName + T("Prediction")));
    //targetPredictor->getOutputVariable()->setName(proteinClass->getMemberVariableName(target));
  }

  builder.addFunction(new MakeProteinFunction(), makeProteinInputs);
}

bool ProteinPredictor::loadFromXml(XmlImporter& importer)
{
  // todo ..
  return Object::loadFromXml(importer);
}

void ProteinPredictor::saveToXml(XmlExporter& exporter) const
{
  ClassPtr type = getClass();
  size_t n = type->getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable variable = getVariable(i);
    if (!variable.isMissingValue())
      exporter.saveVariable(getVariableName(i), variable, getVariableType(i));

    if (i == 0)
    {
     // if (residuePerceptionType)
     //   exporter.saveDynamicType(T("residuePerceptionType"), residuePerceptionType);
     // if (residuePairPerceptionType)
     //   exporter.saveDynamicType(T("residuePairPerceptionType"), residuePairPerceptionType);
    }
  }
}
