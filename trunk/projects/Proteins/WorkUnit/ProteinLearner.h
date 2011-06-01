/*-----------------------------------------.---------------------------------.
| Filename: ProteinLearner.h               | Protein Learner Work Unit       |
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

class ProteinLearner : public WorkUnit
{
public:
  ProteinLearner();

  virtual Variable run(ExecutionContext& context);

protected:
  friend class ProteinLearnerClass;

  File inputDirectory;
  File supervisionDirectory;
  size_t maxProteins;
  ProteinPredictorParametersPtr parameters;

  std::vector<ProteinTarget> proteinTargets;
  size_t numStacks;

  File predictionDirectory;
  File learnedModelFile;

  FunctionPtr createPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const;
  FunctionPtr createOneStackPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const;
  ContainerPtr loadProteinPairs(ExecutionContext& context, const String& subDirectoryName) const;
  bool savePredictionsToDirectory(ExecutionContext& context, FunctionPtr predictor, ContainerPtr proteinPairs, const File& predictionDirectory) const;
  ScoreObjectPtr selectScoresFromTargets(EvaluatorPtr evaluator, ScoreObjectPtr scores) const;
};

  
class ProteinLearnerObjectiveFunction : public SimpleUnaryFunction
{
public:
  ProteinLearnerObjectiveFunction() : SimpleUnaryFunction(numericalProteinFeaturesParametersClass, doubleType, T("ProteinLearnerObjectiveFunction evalued")) {}
  
  Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    WorkUnitPtr wu = new ProteinLearner();
    wu->parseArguments(context, T("-s ./../../projects/boinc.run.montefiore.ulg.ac.be_evo/supervision -i ./../../projects/boinc.run.montefiore.ulg.ac.be_evo/predicted -p \"numerical(") + inputs->toString() + T(",sgd)\" -t ss3 -n 1"));
    ScoreObjectPtr scoreObject = (wu->run(context)).getObjectAndCast<ScoreObject>();
    return scoreObject->getScoreToMinimize();
  }
};  
  
}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
