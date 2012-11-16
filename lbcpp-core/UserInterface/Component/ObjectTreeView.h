/*-----------------------------------------.---------------------------------.
| Filename: ObjectTreeView.h               | Object Tree component           |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 12:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_COMPONENTS_VARIABLE_TREE_H_
# define EXPLORER_COMPONENTS_VARIABLE_TREE_H_

# include <lbcpp/UserInterface/ObjectComponent.h>
# include "GenericTreeView.h"

namespace lbcpp
{

class ObjectTreeView : public GenericTreeView
{
public:
  ObjectTreeView(const ObjectPtr& object, const string& name, bool makeRootNodeVisible);
  
  juce_UseDebuggingNewOperator

protected:
  virtual GenericTreeViewItem* createItem(const ObjectPtr& object, const string& name);
  virtual bool mightHaveSubObjects(const ObjectPtr& object);
  virtual std::vector< std::pair<string, ObjectPtr> > getSubObjects(const ObjectPtr& object);
};

}; /* namespace lbcpp */

#endif // !EXPLORER_COMPONENTS_VARIABLE_TREE_H_
