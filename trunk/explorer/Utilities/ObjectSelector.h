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

  virtual void selectionChangedCallback(const std::vector<Variable>& selectedVariables) = 0;
};

class ObjectSelector
{
public:
  void addCallback(ObjectSelectorCallback& callback)
    {callbacks.push_back(&callback);}

  void sendSelectionChanged(const std::vector<Variable>& selectedVariables)
  {
    for (size_t i = 0; i < callbacks.size(); ++i)
      callbacks[i]->selectionChangedCallback(selectedVariables);
  }

protected:
  std::vector<ObjectSelectorCallback* > callbacks;
};

}; /* namespace lbcpp */

#endif // !EXPLORER_UTILITIES_OBJECT_SELECTOR_H_
