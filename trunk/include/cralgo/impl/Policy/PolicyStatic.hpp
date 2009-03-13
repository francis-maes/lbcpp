/*-----------------------------------------.---------------------------------.
| Filename: PolicyStatic.hpp               | Static Policy Interface         |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_STATIC_H_
# define CRALGO_IMPL_POLICY_STATIC_H_

# include "../../CRAlgorithm.h"
# include "../Object.hpp"

namespace cralgo {
namespace impl {

template<class ExactType>
struct Policy : public Object<ExactType>
{
  typedef Object<ExactType> BaseClass;
  
  void policyEnter(CRAlgorithmPtr crAlgorithm) {}

  VariablePtr policyChoose(ChoosePtr choose) {assert(false); return VariablePtr();}
  void policyReward(double reward) {}
  
  void policyLeave() {}
  
  size_t getNumResults() const
    {return 0;}
    
  ObjectPtr getResult(size_t i) const
    {assert(false); return ObjectPtr();}
    
  std::string toString() const
  {
    size_t n = BaseClass::_this().getNumResults();
    std::string res = cralgo::toString(n) + " result(s):\n";
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr result = BaseClass::_this().getResult(i);
      assert(result);
      res += "\t" + cralgo::toString(i) + ")\t" + result->getName() + "\t" + cralgo::toString(BaseClass::_this().getResult(i)) + "\n";
    }
    return res;
  }
    
protected:
  CRAlgorithmPtr currentCRAlgorithm;
};

template<class ExactType, class DecoratedType>
struct DecoratorPolicy : public Policy<ExactType>
{
  DecoratorPolicy(const DecoratedType& decorated)
    : decorated(decorated) {}
  
  DecoratedType decorated;

  void policyEnter(CRAlgorithmPtr crAlgorithm)
    {decorated.policyEnter(crAlgorithm);}
    
  VariablePtr policyChoose(ChoosePtr choose)
    {return decorated.policyChoose(choose);}
    
  void policyReward(double reward)
    {decorated.policyReward(reward);}
    
  void policyLeave()
    {decorated.policyLeave();}

  size_t getNumResults() const
    {return decorated.getNumResults();}
    
  ObjectPtr getResult(size_t i) const
    {return decorated.getResult(i);}
};

template<class ExactType, class DecoratedType>
struct EpisodicDecoratorPolicy : public DecoratorPolicy<ExactType, DecoratedType>
{
  typedef DecoratorPolicy<ExactType, DecoratedType> BaseClass;
  
  EpisodicDecoratorPolicy(const DecoratedType& decorated)
    : BaseClass(decorated), inclusionLevel(0) {}
  
  void episodeEnter(CRAlgorithmPtr crAlgorithm)
    {assert(false);}
  void episodeLeave()
    {assert(false);}
  
  void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    BaseClass::policyEnter(crAlgorithm);
    if (inclusionLevel == 0)
      BaseClass::_this().episodeEnter(crAlgorithm);
    ++inclusionLevel;
  }
  
  void policyLeave()
  {
    BaseClass::policyLeave();    
    assert(inclusionLevel > 0);
    --inclusionLevel;
    if (inclusionLevel == 0)
      BaseClass::_this().episodeLeave();
  }
  
private:
  size_t inclusionLevel;
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_STATIC_H_
