/*-----------------------------------------.---------------------------------.
| Filename: Perturbator.h                  | Perturbator base classes        |
| Author  : Francis Maes                   |                                 |
| Started : 20/10/2012 22:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_PERTURBATOR_H_
# define LBCPP_ML_PERTURBATOR_H_

# include <ml/Domain.h>

namespace lbcpp
{

class Perturbator : public Object
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain) {}
  virtual ObjectPtr sample(ExecutionContext& context, const ObjectPtr& object) const = 0;
};

typedef ReferenceCountedObjectPtr<Perturbator> PerturbatorPtr;

class BinaryPerturbator : public Perturbator
{
public:
  virtual ObjectPtr sample(ExecutionContext& context, const ObjectPtr& object) const
  {
    PairPtr pair = object.staticCast<Pair>();
    std::pair<ObjectPtr, ObjectPtr> res = samplePair(context, pair->getFirst(), pair->getSecond());
    return new Pair(res.first, res.second);
  }

  virtual std::pair<ObjectPtr, ObjectPtr> samplePair(ExecutionContext& context, const ObjectPtr& object1, const ObjectPtr& object2) const = 0;
};

typedef ReferenceCountedObjectPtr<BinaryPerturbator> BinaryPerturbatorPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PERTURBATOR_H_
