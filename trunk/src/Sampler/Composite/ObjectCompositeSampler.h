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

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples) const
  {
    size_t n = samples->getNumElements();
    size_t numSamplers = samplers.size();
    subInputs.resize(numSamplers, inputs);
    subSamples.resize(numSamplers);
    for (size_t i = 0; i < numSamplers; ++i)
      subSamples[i] = new VariableVector(0); // todo: static typing for better vector implementation

    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr object = samples->getElement(i).getObject();
      jassert(object->getNumVariables() >= samplers.size());
      for (size_t j = 0; j < numSamplers; ++j)
        subSamples[j].staticCast<VariableVector>()->append(object->getVariable(j));
    }
  }

protected:
  friend class ObjectCompositeSamplerClass;

  ClassPtr objectClass;
};

typedef ReferenceCountedObjectPtr<ObjectCompositeSampler> ObjectCompositeSamplerPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_OBJECT_H_
