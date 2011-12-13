/*-----------------------------------------.---------------------------------.
| Filename: TuningSAParametersWorkUnit.h   | TuningSAParametersWorkUnit      |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 13, 2011  11:15:08 AM      |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_TUNINGSAPARAMETERSWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_TUNINGSAPARAMETERSWORKUNIT_H_

# include "precompiled.h"
# include "../RosettaUtils.h"
# include "../Sampler/GeneralProteinMoverSampler.h"

namespace lbcpp
{

class TuningSAParametersWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
#ifdef LBCPP_PROTEIN_ROSETTA

    rosettaInitialization(context, true);

    File referenceFile = context.getFile(referenceDirectory);
    if (!referenceFile.exists())
    {
      context.errorCallback(T("References' directory not found."));
      return Variable();
    }




    return Variable();
#else
    jassert(false);
    return false;
#endif // LBCPP_PROTEIN_ROSETTA
  }

protected:
  friend class TuningSAParametersWorkUnitClass;

  String referenceDirectory;
  int numIterations;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_TUNINGSAPARAMETERSWORKUNIT_H_
