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
/*# include "../Perception/ProteinPerception.h"
# include "../Inference/ProteinInferenceFactory.h"
# include "../Inference/ProteinInference.h"
# include "../Inference/ContactMapInference.h"
# include "../Evaluator/ProteinEvaluator.h"*/
# include "../Frame/ProteinFrame.h"

namespace lbcpp
{

class SandBox : public WorkUnit
{
public:
  SandBox() : maxProteins(0) {}

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

    return true;
  }

protected:
  friend class SandBoxClass;

  File inputDirectory;
  File supervisionDirectory;
  size_t maxProteins;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_WORK_UNIT_SAND_BOX_H_
