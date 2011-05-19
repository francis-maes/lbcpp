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

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
  {
    size_t n = samples->getNumElements();
    size_t numSamplers = samplers.size();
    subInputs.resize(numSamplers, inputs);
    subSamples.resize(numSamplers);
    for (size_t i = 0; i < numSamplers; ++i)
      subSamples[i] = new DenseDoubleVector(0, 0.0);

    for (size_t i = 0; i < n; ++i)
    {
      DoubleMatrixPtr matrix = samples->getElement(i).getObjectAndCast<DoubleMatrix>();
      jassert(matrix->getNumRows() == numRows && matrix->getNumColumns() == numColumns);
      for (size_t j = 0; j < numSamplers; ++j)
        subSamples[j].staticCast<DenseDoubleVector>()->appendValue(matrix->getValue(j / numColumns, j % numColumns));
    }
  }

protected:
  friend class IndependentDoubleMatrixSamplerClass;

  size_t numRows;
  size_t numColumns;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_COMPOSITE_INDEPENDENT_DOUBLE_VECTOR_H_
