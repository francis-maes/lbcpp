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
# include "../Data/ProteinFunctions.h"
# include "../Predictor/ProteinPredictor.h"

namespace lbcpp
{

class MyProteinPredictorParameters : public NumericalProteinPredictorParameters
{
public:
  MyProteinPredictorParameters(size_t maxLearningIterations)
    : maxLearningIterations(maxLearningIterations)
  {
  }

  virtual FunctionPtr learningMachine(ProteinTarget target) const
  {
    StochasticGDParametersPtr parameters = new StochasticGDParameters(constantIterationFunction(0.1));
    parameters->setMaxIterations(maxLearningIterations);

    parameters->setEvaluator(classificationAccuracyEvaluator()); // tmp
    return linearLearningMachine(parameters);
  }

protected:
  size_t maxLearningIterations;
};

class SandBox : public WorkUnit
{
public:
  SandBox() : maxProteins(0), numFolds(7), maxLearningIterations(100) {}

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

    // make train and test proteins
    ContainerPtr trainingProteins = proteins->invFold(0, numFolds);
    ContainerPtr testingProteins = proteins->fold(0, numFolds);
    context.informationCallback(String((int)trainingProteins->getNumElements()) + T(" training proteins, ") +
                               String((int)testingProteins->getNumElements()) + T(" testing proteins"));
    
    // create predictor
    ProteinPredictorParametersPtr parameters = new MyProteinPredictorParameters(maxLearningIterations);
    ProteinPredictorPtr predictor = new ProteinPredictor(parameters);
    //predictor->addTarget(sa20Target);
    //predictor->addTarget(drTarget);
    predictor->addTarget(ss3Target);
    //predictor->addTarget(ss8Target);
  
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

protected:
  friend class SandBoxClass;

  File inputDirectory;
  File supervisionDirectory;
  size_t maxProteins;
  size_t numFolds;
  size_t maxLearningIterations;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
