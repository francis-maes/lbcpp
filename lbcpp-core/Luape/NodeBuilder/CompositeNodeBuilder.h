/*-----------------------------------------.---------------------------------.
| Filename: CompositeNodeBuilder.h         | Composite Node Builder          |
| Author  : Francis Maes                   |                                 |
| Started : 03/01/2012 17:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_COMPOSITE_H_
# define LBCPP_LUAPE_NODE_BUILDER_COMPOSITE_H_

# include <lbcpp/Luape/ExpressionBuilder.h>

namespace lbcpp
{

class CompositeNodeBuilder : public ExpressionBuilder
{
public:
  CompositeNodeBuilder(const std::vector<ExpressionBuilderPtr>& builders)
    : builders(builders) {}
  CompositeNodeBuilder(ExpressionBuilderPtr builder1, ExpressionBuilderPtr builder2)
    : builders(2) {builders[0] = builder1; builders[1] = builder2;}
  CompositeNodeBuilder() {}
 
  virtual void buildNodes(ExecutionContext& context, const ExpressionDomainPtr& function, size_t maxCount, std::vector<ExpressionPtr>& res)
  {
    for (size_t i = 0; i < builders.size(); ++i)
    {
      builders[i]->buildNodes(context, function, maxCount ? maxCount - res.size() : 0, res);
      if (maxCount && res.size() >= maxCount)
        break;
    }
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const
  {
    const ReferenceCountedObjectPtr<CompositeNodeBuilder>& target = t.staticCast<CompositeNodeBuilder>();
    target->builders.resize(builders.size());
    for (size_t i = 0; i < builders.size(); ++i)
      target->builders[i] = builders[i]->cloneAndCast<ExpressionBuilder>(context);
  }

protected:
  friend class CompositeNodeBuilderClass;

  std::vector<ExpressionBuilderPtr> builders;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_COMPOSITE_H_
