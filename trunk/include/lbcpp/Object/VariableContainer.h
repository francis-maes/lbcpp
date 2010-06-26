/*-----------------------------------------.---------------------------------.
| Filename: VariableContainer.h            | Variable Container base class   |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VARIABLE_CONTAINER_H_
# define LBCPP_OBJECT_VARIABLE_CONTAINER_H_

# include "Variable.h"

namespace lbcpp
{

class VariableContainer : public Object
{
public:
  bool empty() const
    {return size() == 0;}

  virtual size_t size() const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VARIABLE_CONTAINER_H_
