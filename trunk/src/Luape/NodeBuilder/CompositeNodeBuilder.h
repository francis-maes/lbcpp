/*-----------------------------------------.---------------------------------.
| Filename: CompositeNodeBuilder.h         | Composite Node Builder          |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 17:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_COMPOSITE_H_
# define LBCPP_LUAPE_NODE_BUILDER_COMPOSITE_H_

# include <lbcpp/Luape/LuapeNodeBuilder.h>

namespace lbcpp
{

class CompositeNodeBuilder : public LuapeNodeBuilder
{
public:
  CompositeNodeBuilder(const std::vector<LuapeNodeBuilderPtr>& builders)
    : builders(builders) {}
  CompositeNodeBuilder(LuapeNodeBuilderPtr builder1, LuapeNodeBuilderPtr builder2)
    : builders(2) {builders[0] = builder1; builders[1] = builder2;}
  CompositeNodeBuilder() {}
 
  virtual void buildNodes(ExecutionContext& context, const LuapeInferencePtr& function, size_t maxCount, std::vector<LuapeNodePtr>& res)
  {
    for (size_t i = 0; i < builders.size(); ++i)
    {
      builders[i]->buildNodes(context, function, maxCount ? maxCount - res.size() : 0, res);
      if (maxCount && res.size() >= maxCount)
        break;
    }
  }

protected:
  friend class CompositeNodeBuilderClass;

  std::vector<LuapeNodeBuilderPtr> builders;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_COMPOSITE_H_
