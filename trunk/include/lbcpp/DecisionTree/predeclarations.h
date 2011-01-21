/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Function predeclarations        |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 20:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_PREDECLARATIONS_H_
# define LBCPP_DECISION_TREE_PREDECLARATIONS_H_

# include "../Data/predeclarations.h"

namespace lbcpp
{
  
  class RTree;
  typedef ReferenceCountedObjectPtr<RTree> RTreePtr;
  
  class RTreeInference;
  typedef ReferenceCountedObjectPtr<RTreeInference> RTreeInferencePtr;
  
  class RTreeInferenceLearner;
  
}; /* namespace lbcpp */

#endif //!LBCPP_DECISION_TREE_PREDECLARATIONS_H_
