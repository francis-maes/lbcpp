/*-----------------------------------------.---------------------------------.
| Filename: SingletonNodeBuilder.h         | Singleton Node Builder          |
| Author  : Francis Maes                   |                                 |
| Started : 15/12/2011 12:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_SINGLETON_H_
# define LBCPP_LUAPE_NODE_BUILDER_SINGLETON_H_

# include <lbcpp/Luape/LuapeNodeBuilder.h>

namespace lbcpp
{

class SingletonNodeBuilder : public LuapeNodeBuilder
{
public:
  SingletonNodeBuilder(const LuapeNodePtr& node = LuapeNodePtr())
    : node(node) {}

  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res)
    {res.push_back(node);}

protected:
  friend class SingletonNodeBuilderClass;

  LuapeNodePtr node;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_SINGLETON_H_
