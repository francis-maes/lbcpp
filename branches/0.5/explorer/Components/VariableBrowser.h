/*-----------------------------------------.---------------------------------.
| Filename: VariableBrowser.h              | The Variable Browser            |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_
# define EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_

# include "VariableProxyComponent.h"
# include "../Utilities/PropertyListDisplayComponent.h"
# include <lbcpp/UserInterface/ComponentWithPreferedSize.h>
# include <lbcpp/UserInterface/VariableSelector.h>

class VariableBrowserContent;

namespace lbcpp
{

class VariableBrowser : public ViewportComponent
{
public:
  VariableBrowser(const Variable& variable, Component* selector);

  virtual void resized();

private:
  VariableBrowserContent* getContent() const;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_

