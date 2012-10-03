/*-----------------------------------------.---------------------------------.
| Filename: SingletonNodeBuilder.h         | Singleton Node Builder          |
| Author  : Francis Maes                   |                                 |
| Started : 15/12/2011 12:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_SINGLETON_H_
# define LBCPP_LUAPE_NODE_BUILDER_SINGLETON_H_

# include <lbcpp/Luape/ExpressionBuilder.h>

namespace lbcpp
{

class SingletonNodeBuilder : public ExpressionBuilder
{
public:
  SingletonNodeBuilder(const ExpressionPtr& node = ExpressionPtr())
    : node(node) {}

  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<ExpressionPtr>& res)
    {res.push_back(node);}

protected:
  friend class SingletonNodeBuilderClass;

  ExpressionPtr node;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_SINGLETON_H_
