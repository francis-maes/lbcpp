/*-----------------------------------------.---------------------------------.
| Filename: PerceptionMaths.h              | Perception Math Functions       |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 16:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_PERCEPTION_MATHS_H_
# define LBCPP_DATA_PERCEPTION_MATHS_H_

# include "Perception.h"

namespace lbcpp
{

// Const-unary operations
extern size_t l0norm(const ObjectPtr& object);
extern size_t l0norm(const PerceptionPtr& perception, const Variable& input);

extern double l1norm(const ObjectPtr& object);
extern double l1norm(const PerceptionPtr& perception, const Variable& input);

extern double sumOfSquares(const ObjectPtr& object);
extern double sumOfSquares(const PerceptionPtr& perception, const Variable& input);

inline double l2norm(const ObjectPtr& object)
  {return object ? sqrt(sumOfSquares(object)) : 0.0;}

inline double l2norm(const PerceptionPtr& perception, const Variable& input)
  {return sqrt(sumOfSquares(perception, input));}

// Unary operations
extern void multiplyByScalar(const ObjectPtr& object, double scalar);

// Binary operations
extern double dotProduct(const ObjectPtr& object, const PerceptionPtr& perception, const Variable& input);
extern double dotProduct(const ObjectPtr& object1, const ObjectPtr& object2);

extern void addWeighted(ObjectPtr& target, const ObjectPtr& source, double weight);
extern void addWeighted(ObjectPtr& target, const PerceptionPtr& perception, const Variable& input, double weight);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_MATHS_H_
