/*-----------------------------------------.---------------------------------.
| Filename: SandBox.h                      | Sand Box Work Unit              |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2011 19:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
# define LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_

# include <lbcpp/lbcpp.h>
# include "../Data/Protein.h"
# include "../Frame/ProteinFrame.h"

namespace lbcpp
{

// protein, (targetIndex, targetValue)* -> protein
class MakeProteinFunction : public Function
{
public:
  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)proteinClass : anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("protein");
    outputShortName = T("prot");
    return proteinClass;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ProteinPtr inputProtein = inputs[0].getObjectAndCast<Protein>();
    if (!inputProtein)
      return Variable::missingValue(proteinClass);

    ProteinPtr res = inputProtein->cloneAndCast<Protein>();
    for (size_t i = 1; i < numInputs; i += 2)
    {
      size_t targetIndex = (size_t)inputs[i].getInteger();
      const Variable& target = inputs[i + 1];
      if (target.exists())
      {
        jassert(targetIndex < proteinClass->getNumMemberVariables());
        res->setVariable(targetIndex, target);
      }
    }
    return res;
  }
};

// protein -> variable
class GetProteinTargetFunction : public SimpleUnaryFunction
{
public:
  GetProteinTargetFunction(ProteinTarget target = noTarget)
    : SimpleUnaryFunction(proteinClass, proteinClass->getMemberVariableType(target), T("Target")), target(target) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return input.getObjectAndCast<Protein>()->getTargetOrComputeIfMissing(target);}

protected:
  ProteinTarget target;
};

// protein, protein -> protein
class ProteinPredictor : public CompositeFunction
{
public:
  ProteinPredictor(ProteinPredictorParametersPtr factory = ProteinPredictorParametersPtr())
    : factory(factory) {}

  void addTarget(ProteinTarget target)
  {
    FunctionPtr targetPredictor = factory->createTargetPredictor(target);
    jassert(targetPredictor);
    targetPredictors.push_back(std::make_pair(target, targetPredictor));
  }

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input = builder.addInput(proteinClass, T("input"));
    size_t supervision = builder.addInput(proteinClass, T("supervision"));
    size_t residueFeatures = builder.addFunction(factory->createResidueVectorPerception(), input);
    
    std::vector<size_t> makeProteinInputs;
    makeProteinInputs.push_back(input);
    for (size_t i = 0; i < targetPredictors.size(); ++i)
    {
      ProteinTarget target = targetPredictors[i].first;
      String targetName = proteinClass->getMemberVariableName(target);
      FunctionPtr targetPredictor = targetPredictors[i].second;

      size_t targetPredictorInput;
      TypePtr elementsType = Container::getTemplateParameter(proteinClass->getMemberVariableType(target));
      jassert(elementsType);
      if (elementsType->inheritsFrom(enumValueType)) 
        targetPredictorInput = residueFeatures; // label sequence -> residue features
      else
        jassert(false);

      size_t targetSupervision = builder.addFunction(new GetProteinTargetFunction(target), supervision, targetName + T("Supervision"));
      makeProteinInputs.push_back(builder.addConstant((int)target));
      makeProteinInputs.push_back(builder.addFunction(targetPredictor, targetPredictorInput, targetSupervision, targetName + T("Prediction")));
    }
    builder.addFunction(new MakeProteinFunction(), makeProteinInputs);
  }

protected:
  ProteinPredictorParametersPtr factory;
  std::vector< std::pair<ProteinTarget, FunctionPtr> > targetPredictors;
};

typedef ReferenceCountedObjectPtr<ProteinPredictor> ProteinPredictorPtr;

////////////////////////////////////////////////////////////////


class MyProteinPredictorParameters : public NumericalProteinPredictorParameters
{
public:
  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    StochasticGDParametersPtr parameters = new StochasticGDParameters();
    parameters->setMaxIterations(100);
    return linearLearningMachine(parameters);
  }
};

class SandBox : public WorkUnit
{
public:
  SandBox() : maxProteins(0), numFolds(7) {}

  virtual Variable run(ExecutionContext& context)
  {
    // load proteins
    if (!supervisionDirectory.exists() || !supervisionDirectory.isDirectory())
    {
      context.errorCallback(T("Invalid supervision directory"));
      return false;
    }
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteins, T("Loading"));
    if (!proteins)
      return false;
    jassert(proteins->getElementsType() == pairClass(proteinClass, proteinClass));

    // make train and test proteins
    ContainerPtr trainingProteins = proteins->invFold(0, numFolds);
    ContainerPtr testingProteins = proteins->fold(0, numFolds);
    context.informationCallback(String((int)trainingProteins->getNumElements()) + T(" training proteins, ") +
                               String((int)testingProteins->getNumElements()) + T(" testing proteins"));
    
    // create predictor
    ProteinPredictorParametersPtr parameters = new MyProteinPredictorParameters();
    ProteinPredictorPtr predictor = new ProteinPredictor(parameters);
    predictor->addTarget(ss3Target);
    predictor->addTarget(ss8Target);
    
    /*

    VectorPtr trainingExamples = makeSecondaryStructureExamples(context, trainingProteins);
    VectorPtr testingExamples = makeSecondaryStructureExamples(context, testingProteins);
    context.informationCallback(String((int)trainingExamples->getNumElements()) + T(" training examples, ") +
                               String((int)testingExamples->getNumElements()) + T(" testing examples"));


    StochasticGDParametersPtr parameters = new StochasticGDParameters();
    parameters->setEvaluator(classificationAccuracyEvaluator());

    FunctionPtr classifier = linearLearningMachine(parameters);*/
    if (!predictor->train(context, trainingProteins, ContainerPtr(), T("Training"), true))
      return false;

    // evaluate on training data
    if (!predictor->evaluate(context, trainingProteins, new ProteinEvaluator(), T("Evaluate on training data")))
      return false;
    
    // evaluate on testing data
    if (!predictor->evaluate(context, testingProteins, new ProteinEvaluator(), T("Evaluate on testing data")))
      return false;

    return true;
  }
#if 0
  VectorPtr makeSecondaryStructureExamples(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    FunctionPtr computeResidueFeaturesFunction = proteinResidueFeaturesVectorFunction();
    if (!computeResidueFeaturesFunction->initialize(context, (TypePtr)proteinClass))
      return VectorPtr();

    ClassPtr featuresClass = Container::getTemplateParameter(computeResidueFeaturesFunction->getOutputType());
    jassert(featuresClass);
    TypePtr examplesType = pairClass(featuresClass, secondaryStructureElementEnumeration);
    VectorPtr res = vector(examplesType);
    size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      PairPtr proteinPair = proteins->getElement(i).getObjectAndCast<Pair>();
      jassert(proteinPair);
      const ProteinPtr& inputProtein = proteinPair->getFirst().getObjectAndCast<Protein>();
      const ProteinPtr& supervisionProtein = proteinPair->getSecond().getObjectAndCast<Protein>();
      jassert(inputProtein && supervisionProtein);
      VectorPtr residueFeatures = computeResidueFeaturesFunction->compute(context, inputProtein).getObjectAndCast<Vector>();
      makeSecondaryStructureExamples(residueFeatures, supervisionProtein, res);
    }
    return res;    
  }

  void makeSecondaryStructureExamples(const VectorPtr& residueFeatures, const ProteinPtr& supervisionProtein, const VectorPtr& res) const
  {
    jassert(residueFeatures);
    VectorPtr secondaryStructure = supervisionProtein->getSecondaryStructure();
    if (secondaryStructure)
    {
      size_t n = secondaryStructure->getNumElements();
      jassert(residueFeatures->getNumElements() == n);
      for (size_t i = 0; i < n; ++i)
      {
        Variable ss3 = secondaryStructure->getElement(i);
        if (ss3.exists())
          res->append(new Pair(res->getElementsType(), residueFeatures->getElement(i), ss3));
      }
    }
  }
#endif // 0

protected:
  friend class SandBoxClass;

  File inputDirectory;
  File supervisionDirectory;
  size_t maxProteins;
  size_t numFolds;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
