/*-----------------------------------------.---------------------------------.
| Filename: IndependentDoubleMatrixSampler.h| Samples matrices in R^{n*m}    |
| Author  : Francis Maes                   |                                 |
| Started : 14/05/2011 13:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DOUBLE_MATRIX_H_
# define LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DOUBLE_MATRIX_H_

# include <lbcpp/Sampler/Sampler.h>
# include <lbcpp/Data/Matrix.h>

namespace lbcpp
{

class IndependentDoubleMatrixSampler : public CompositeSampler
{
public:
  IndependentDoubleMatrixSampler(size_t numRows, size_t numColumns, const SamplerPtr& elementSamplerModel)
    : CompositeSampler(numRows * numColumns), numRows(numRows), numColumns(numColumns)
  {
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i] = elementSamplerModel->cloneAndCast<Sampler>();
  }

  IndependentDoubleMatrixSampler() {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DoubleMatrixPtr res = new DoubleMatrix(doubleType, numRows, numColumns);
    size_t index = 0;
    for (size_t i = 0; i < numRows; ++i)
      for (size_t j = 0; j < numColumns; ++j, ++index)
        res->setValue(i, j, samplers[i]->sample(context, random).toDouble());
    return res;
  }

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset)
  {
    size_t n = samplers.size();

    std::vector< std::vector<Variable> > subDatasets(n);
    for (size_t i = 0; i < n; ++i)
      subDatasets[i].resize(dataset.size());

    for (size_t i = 0; i < dataset.size(); ++i)
    {
      const DoubleMatrixPtr& matrix = dataset[i].getObjectAndCast<DoubleMatrix>();
      jassert(matrix->getNumRows() == numRows && matrix->getNumColumns() == numColumns);
      for (size_t j = 0; j < n; ++j)
        subDatasets[j][i] = matrix->getValue(j / numColumns, j % numColumns);
    }

    for (size_t i = 0; i < n; ++i)
      samplers[i]->learn(context, subDatasets[i]);
  }

protected:
  friend class IndependentDoubleMatrixSamplerClass;

  size_t numRows;
  size_t numColumns;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DOUBLE_VECTOR_H_
