/*-----------------------------------------.---------------------------------.
| Filename: Predicate.h                    | Predicate : functions from      |
| Author  : Francis Maes                   |   Variables to booleans         |
| Started : 06/07/2010 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PREDICATE_H_
# define LBCPP_OBJECT_PREDICATE_H_

# include <lbcpp/Object/Variable.h>

namespace lbcpp
{

class Predicate : public Object
{
public:
  virtual bool compute(const Variable& value) const = 0;
};

typedef ReferenceCountedObjectPtr<Predicate> PredicatePtr;

extern PredicatePtr logicalOr(PredicatePtr predicate1, PredicatePtr predicate2);
extern PredicatePtr logicalAnd(PredicatePtr predicate1, PredicatePtr predicate2);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PREDICATE_H_
