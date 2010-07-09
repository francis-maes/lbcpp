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
class SymmetricMatrix : public Container 
{
public:
  SymmetricMatrix(TypePtr contentType, size_t initialSize = 0) 
    : dimension(0), values(new Vector(contentType)) {}

  SymmetricMatrix() {}

  size_t size() const
    {return dimension;}
  
  void setValue(size_t i, size_t j, const Variable& value)
    {}

  Variable getValue(size_t i, size_t j) const
    {return Variable();}

private:
  VectorPtr values;
  size_t dimension;
};

typedef ReferenceCountedObjectPtr<SymmetricMatrix> SymmetricMatrixPtr;

extern ClassPtr symmetricMatrixClass(TypePtr elementsType);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_SYMMETRIC_MATRIX_H_
