#ifndef LBCPP_PROTEIN_EXHAUSTIVE_DISULFIDE_PATTERN_H_
# define LBCPP_PROTEIN_EXHAUSTIVE_DISULFIDE_PATTERN_H_

# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ExhaustiveDisulfidePatternFunction : public SimpleUnaryFunction
{
public:
  ExhaustiveDisulfidePatternFunction(double threshold = 0.5, TypePtr elementsType = probabilityType, size_t minimumDistanceFromDiagonal = 1)
    : SimpleUnaryFunction(symmetricMatrixClass(elementsType), symmetricMatrixClass(elementsType), T("ExhaustivePattern")),
      threshold(threshold), minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) 
    {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DoubleSymmetricMatrixPtr matrix = input.getObjectAndCast<DoubleSymmetricMatrix>(context);
    if (!matrix)
      return Variable::missingValue(getOutputType());

    const size_t dimension = matrix->getDimension();
    if (dimension <= minimumDistanceFromDiagonal)
      return matrix;

    //std::cout << "Starting matrix:" << std::endl << matrix->toString() << std::endl;
    
    CharSymmetricMatrixPtr initialMask = createInitialMask(context, matrix);
    //std::cout << "Starting Pattern:" << std::endl << initialMask->toString() << std::endl;

    std::pair<double, CharSymmetricMatrixPtr> bestMask = findBestMask(context, matrix, initialMask, 0.0);
    //std::cout << "Score: " << bestMask.first << std::endl;
    SymmetricMatrixPtr res = makeResult(context, matrix, bestMask.second);
    //std::cout << "Pattern: " << std::endl << bestMask.second->toString() << std::endl;
    return res;
  }
  
protected:
  enum {notBridged = -1, unknown = 0, bridged = 1};

  double threshold;
  size_t minimumDistanceFromDiagonal;

  SymmetricMatrixPtr createInitialMask(ExecutionContext& context, const DoubleSymmetricMatrixPtr& matrix) const
  {
    const size_t dimension = matrix->getDimension();
    CharSymmetricMatrixPtr res = new CharSymmetricMatrix(integerType, dimension, notBridged);
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
        if (matrix->getValue(i, j) > threshold)
          res->setValue(i, j, unknown);
    return res;
  }

  std::pair<double, CharSymmetricMatrixPtr> findBestMask(ExecutionContext& context, const DoubleSymmetricMatrixPtr& matrix, const CharSymmetricMatrixPtr& mask, double score) const
  {
    std::pair<double, CharSymmetricMatrixPtr> bestMask = std::make_pair(score, mask);
    
    const size_t dimension = mask->getDimension();
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
        if (mask->getValue(i, j) == unknown)
        {
          CharSymmetricMatrixPtr clone = mask->cloneAndCast<CharSymmetricMatrix>(context);
          setBridge(clone, i, j);
          std::pair<double, CharSymmetricMatrixPtr> res = findBestMask(context, matrix, clone, score + matrix->getValue(i, j));
          if (res.first > score)
            bestMask = res;
        }
    
    return bestMask;
  }

  void setBridge(const CharSymmetricMatrixPtr& mask, size_t x, size_t y) const
  {
    const size_t dimension = mask->getDimension();
    for (size_t i = 0; i < dimension; ++i)
    {
      mask->setValue(x, i, notBridged);
      mask->setValue(y, i, notBridged);
    }
    mask->setValue(x, y, bridged);
  }

  SymmetricMatrixPtr makeResult(ExecutionContext& context, const DoubleSymmetricMatrixPtr& matrix, const CharSymmetricMatrixPtr& mask) const
  {
    DoubleSymmetricMatrixPtr res = matrix->cloneAndCast<DoubleSymmetricMatrix>(context);
    
    const size_t dimension = matrix->getDimension();
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = i; j < dimension; ++j)
      {
        jassert(mask->getValue(i, j) != unknown);
        if (mask->getValue(i, j) == notBridged)
          res->setValue(i, j, 0.f);
      }
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_EXHAUSTIVE_DISULFIDE_PATTERN_H_
