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

class SandBox : public WorkUnit
{
public:
  SandBox() : maxProteins(0), numFolds(7) {}

  virtual Variable run(ExecutionContext& context)
  {
    if (!supervisionDirectory.exists() || !supervisionDirectory.isDirectory())
    {
      context.errorCallback(T("Invalid supervision directory"));
      return false;
    }

    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteins, T("Loading"));
    if (!proteins)
      return false;
    
    ContainerPtr trainingProteins = proteins->invFold(0, numFolds);
    ContainerPtr testingProteins = proteins->fold(0, numFolds);
    context.informationCallback(String((int)trainingProteins->getNumElements()) + T(" training proteins, ") +
                               String((int)testingProteins->getNumElements()) + T(" testing proteins"));
    
    
    VectorPtr trainingExamples = makeSecondaryStructureExamples(context, trainingProteins);
    VectorPtr testingExamples = makeSecondaryStructureExamples(context, testingProteins);
    context.informationCallback(String((int)trainingExamples->getNumElements()) + T(" training examples, ") +
                               String((int)testingExamples->getNumElements()) + T(" testing examples"));


    StochasticGDParametersPtr parameters = new StochasticGDParameters();
    parameters->setEvaluator(classificationAccuracyEvaluator());

    FunctionPtr classifier = linearLearningMachine(parameters);
    if (!classifier->train(context, trainingExamples, ContainerPtr(), T("Training"), true))
      return false;

    // evaluate on training data
    if (!classifier->evaluate(context, trainingExamples, classificationAccuracyEvaluator(), T("Evaluate on training data")))
      return false;
    
    // evaluate on testing data
    if (!classifier->evaluate(context, testingExamples, classificationAccuracyEvaluator(), T("Evaluate on testing data")))
      return false;

    return true;
  }

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

protected:
  friend class SandBoxClass;

  File inputDirectory;
  File supervisionDirectory;
  size_t maxProteins;
  size_t numFolds;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
