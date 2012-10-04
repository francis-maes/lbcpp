/*-----------------------------------------.---------------------------------.
| Filename: InputsNodeBuilder.h            | Inputs node builder             |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 18:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_INPUTS_H_
# define LBCPP_LUAPE_NODE_BUILDER_INPUTS_H_

# include <lbcpp/Luape/ExpressionBuilder.h>

namespace lbcpp
{

class InputsNodeBuilder : public ExpressionBuilder
{
public:
  virtual void buildNodes(ExecutionContext& context, const ExpressionDomainPtr& function, size_t maxCount, std::vector<ExpressionPtr>& res)
  {
    res.reserve(function->getNumInputs() + function->getNumActiveVariables());
    for (size_t i = 0; i < function->getNumInputs(); ++i)
    {
      ExpressionPtr node = function->getInput(i);
      if (node->getType()->isConvertibleToDouble())
        res.push_back(node);
    }
    for (size_t i = 0; i < function->getNumActiveVariables(); ++i)
    {
      ExpressionPtr node = function->getActiveVariable(i);
      if (node->getType()->isConvertibleToDouble())
        res.push_back(node);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_INPUTS_H_

