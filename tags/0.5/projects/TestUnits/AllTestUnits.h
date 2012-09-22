/*-----------------------------------------.---------------------------------.
| Filename: AllTestUnits.h                 | Run all Test Units              |
| Author  : Francis Maes                   |                                 |
| Started : 16/12/2010 15:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_TEST_UNIT_ALL_H_
# define LBCPP_TEST_UNIT_ALL_H_

# include "ExtraTreeTestUnit.h"
# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class AllTestUnits : public CompositeWorkUnit
{
public:
  AllTestUnits() : CompositeWorkUnit(T("AllTestUnits"))
  {
    //workUnits->append(new ExtraTreeTestUnit());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_TEST_UNIT_ALL_H_
