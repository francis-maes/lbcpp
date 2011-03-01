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
  size_t numFolds;
  ProteinPredictorParametersPtr parameters;

  std::vector<ProteinTarget> proteinTargets;
  size_t numStacks;

  File predictionDirectory;

  FunctionPtr createPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const;
  FunctionPtr createOneStackPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const;
  bool savePredictionsToDirectory(ExecutionContext& context, FunctionPtr predictor, ContainerPtr proteinPairs, const File& predictionDirectory) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
