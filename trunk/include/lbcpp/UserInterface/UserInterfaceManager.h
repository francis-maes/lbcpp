/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceManager.h         | User Interface Manager          |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_MANAGER_H_
# define LBCPP_USER_INTERFACE_MANAGER_H_

# include "../Execution/ExecutionContext.h"

namespace lbcpp
{

class UserInterfaceManager
{
public:
  bool hasImage(const string& fileName) const;
  juce::Image* getImage(const string& fileName) const;
  juce::Image* getImage(const string& fileName, int width, int height) const;

  juce::TreeView* createObjectTreeView(ExecutionContext& context, const ObjectPtr& object, const string& name = string::empty,
                                          bool showTypes = true, bool showShortSummaries = true, bool showMissingVariables = false, bool makeRootNodeVisible = true) const;
  juce::TreeView* createExecutionTraceInteractiveTreeView(ExecutionContext& context, ExecutionTracePtr trace, ExecutionContextPtr traceContext) const;
};

extern UserInterfaceManager& userInterfaceManager();

}; /* namespace smode */

#endif // !LBCPP_USER_INTERFACE_MANAGER_H_
