/*-----------------------------------------.---------------------------------.
| Filename: ObjectBrowser.h                | The Object Browser              |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_
# define EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_

# include "../Utilities/PropertyListDisplayComponent.h"
# include <lbcpp/UserInterface/ObjectComponent.h>

class ObjectBrowserContent;

namespace lbcpp
{

class ObjectBrowser : public ViewportComponent
{
public:
  ObjectBrowser(const ObjectPtr& object, Component* selector);

  virtual void resized();

private:
  ObjectBrowserContent* getContent() const;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_VARIABLE_BROWSER_H_

