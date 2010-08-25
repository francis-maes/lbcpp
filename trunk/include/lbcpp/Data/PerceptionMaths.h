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
extern size_t l0norm(ObjectPtr object);
extern size_t l0norm(PerceptionPtr perception, const Variable& input);

extern double l1norm(ObjectPtr object);
extern double l1norm(PerceptionPtr perception, const Variable& input);

extern double sumOfSquares(ObjectPtr object);
extern double sumOfSquares(PerceptionPtr perception, const Variable& input);

inline double l2norm(ObjectPtr object)
  {return object ? sqrt(sumOfSquares(object)) : 0.0;}

inline double l2norm(PerceptionPtr perception, const Variable& input)
  {return sqrt(sumOfSquares(perception, input));}

// Binary operations
extern double dotProduct(ObjectPtr object, PerceptionPtr perception, const Variable& input);

extern void addWeighted(ObjectPtr& target, ObjectPtr source, double weight);
extern void addWeighted(ObjectPtr& object, PerceptionPtr perception, const Variable& input, double weight);

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_PERCEPTION_MATHS_H_
