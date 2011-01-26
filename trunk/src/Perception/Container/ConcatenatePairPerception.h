/*-----------------------------------------.---------------------------------.
| Filename: ConcatenatePairPerception.h    | Concatenate two perceptions     |
| Author  : Francis Maes                   |                                 |
| Started : 26/01/2011 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PERCEPTION_CONTAINER_CONCATENATE_PAIR_H_
# define LBCPP_PERCEPTION_CONTAINER_CONCATENATE_PAIR_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class ConcatenatePairPerception : public Perception
{
public:
  ConcatenatePairPerception(const PerceptionPtr& firstPerception, const PerceptionPtr& secondPerception)
    : firstPerception(firstPerception), secondPerception(secondPerception)
    {computeOutputType();}
  ConcatenatePairPerception() {}

  virtual TypePtr getInputType() const
    {return pairClass(firstPerception->getInputType(), secondPerception->getInputType());}

  virtual void computeOutputType()
  {
    addOutputVariable(T("first"), firstPerception);
    addOutputVariable(T("second"), secondPerception);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    if (pair)
    {
      callback->sense(0, firstPerception, pair->getFirst());
      callback->sense(1, secondPerception, pair->getSecond());
    }
  }

protected:
  friend class ConcatenatePairPerceptionClass;

  PerceptionPtr firstPerception;
  PerceptionPtr secondPerception;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PERCEPTION_CONTAINER_CONCATENATE_PAIR_H_
