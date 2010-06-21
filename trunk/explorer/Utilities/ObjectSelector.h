/*-----------------------------------------.---------------------------------.
| Filename: ObjectSelector.h               | Interface for object selectors  |
| Author  : Francis Maes                   |                                 |
| Started : 14/06/2010 14:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef EXPLORER_UTILITIES_OBJECT_SELECTOR_H_
# define EXPLORER_UTILITIES_OBJECT_SELECTOR_H_

# include "../Components/common.h"

namespace lbcpp
{

class ObjectSelectorCallback
{
public:
  virtual ~ObjectSelectorCallback() {}

  virtual void objectSelectedCallback(ObjectPtr object) = 0; // old
  virtual void selectionChangedCallback(const std::vector<ObjectPtr>& selectedObjects) {} // new
};

class ObjectSelector
{
public:
  void addCallback(ObjectSelectorCallback& callback)
    {callbacks.push_back(&callback);}

  void sendObjectSelected(ObjectPtr object)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->objectSelectedCallback(object);
  }

  void sendSelectionChanged(const std::vector<ObjectPtr>& selectedObjects)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->selectionChangedCallback(selectedObjects);
  }

protected:
  std::vector<ObjectSelectorCallback* > callbacks;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_UTILITIES_OBJECT_SELECTOR_H_
