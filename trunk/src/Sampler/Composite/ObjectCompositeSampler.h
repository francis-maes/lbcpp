/*-----------------------------------------.---------------------------------.
| Filename: ObjectCompositeSampler.h       | Object Composite Sampler        |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_OBJECT_H_
# define LBCPP_SAMPLER_COMPOSITE_OBJECT_H_

# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

class ObjectCompositeSampler : public CompositeSampler
{
public:
  ObjectCompositeSampler(ClassPtr objectClass, const std::vector<SamplerPtr>& variableSamplers)
    : CompositeSampler(variableSamplers), objectClass(objectClass) {}
  ObjectCompositeSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    ObjectPtr res = Object::create(objectClass);
    for (size_t i = 0; i < samplers.size(); ++i)
      res->setVariable(i, samplers[i]->sample(context, random, inputs));
    return res;
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    size_t n = samplers.size();
    std::vector< std::vector<Variable> > subDatasets(n);
    for (size_t i = 0; i < n; ++i)
      subDatasets[i].resize(dataset.size());

    for (size_t i = 0; i < dataset.size(); ++i)
    {
      const ObjectPtr& object = dataset[i].getObject();
      jassert(object->getNumVariables() >= samplers.size());
      for (size_t j = 0; j < n; ++j)
        subDatasets[j][i] = object->getVariable(j);
    }

    for (size_t i = 0; i < n; ++i)
      samplers[i]->learn(context, subDatasets[i]);
  }

protected:
  friend class ObjectCompositeSamplerClass;

  ClassPtr objectClass;
};

typedef ReferenceCountedObjectPtr<ObjectCompositeSampler> ObjectCompositeSamplerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_OBJECT_H_
