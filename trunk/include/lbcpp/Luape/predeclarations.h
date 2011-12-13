/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | Luape Predeclarations           |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 10:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_PREDECLARATIONS_H_
# define LBCPP_LUAPE_PREDECLARATIONS_H_

# include "../Core/predeclarations.h"

namespace lbcpp
{

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

class LuapeInputNode;
typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeConstantNode;
typedef ReferenceCountedObjectPtr<LuapeConstantNode> LuapeConstantNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

class LuapeTestNode;
typedef ReferenceCountedObjectPtr<LuapeTestNode> LuapeTestNodePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_PREDECLARATIONS_H_
