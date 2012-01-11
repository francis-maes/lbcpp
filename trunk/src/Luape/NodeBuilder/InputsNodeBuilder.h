/*-----------------------------------------.---------------------------------.
| Filename: InputsNodeBuilder.h            | Inputs node builder             |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 18:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_INPUTS_H_
# define LBCPP_LUAPE_NODE_BUILDER_INPUTS_H_

# include <lbcpp/Luape/LuapeNodeBuilder.h>

namespace lbcpp
{

class InputsNodeBuilder : public LuapeNodeBuilder
{
public:
  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res)
  {
    res.reserve(function->getNumInputs());
    for (size_t i = 0; i < function->getNumInputs(); ++i)
    {
      LuapeNodePtr node = function->getInput(i);
      if (node->getType() == doubleType)
        res.push_back(node);
    }
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_INPUTS_H_

