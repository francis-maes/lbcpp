/*-----------------------------------------.---------------------------------.
| Filename: SymmetricMatrix.h              | Symmetric Matrix of variables   |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_SYMMETRIC_MATRIX_H_
# define LBCPP_OBJECT_SYMMETRIC_MATRIX_H_

# include "Vector.h"

namespace lbcpp
{

// matrix = container of rows, row = container of Variables
class SymmetricMatrix : public VariableContainer 
{
public:
  SymmetricMatrix() : dimension(0) {}

  size_t size() const
    {return dimension;}
  
private:
  size_t dimension;
  Vector values;
};

typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_SYMMETRIC_MATRIX_H_
