/*-----------------------------------------.---------------------------------.
| Filename: FeatureGeneratorCallbacks.hpp  | Feature Generators Callbacks    |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2011 17:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_CALLBACKS_HPP_H_
# define LBCPP_FEATURE_GENERATOR_CALLBACKS_HPP_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class FillSparseVectorFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  FillSparseVectorFeatureGeneratorCallback(const SparseDoubleVectorPtr& target)
    : target(target) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
  {
    jassert(weight == 1.0);
    vector->appendTo(target, index);
  }

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
  {
    jassert(weight == 1.0);
    featureGenerator->appendTo(inputs, target, index);
  }

private:
  SparseDoubleVectorPtr target;
};

class ComputeL0NormFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  ComputeL0NormFeatureGeneratorCallback() : res(0) {}

  virtual void sense(size_t index, double value)
    {if (value) ++res;}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {if (weight) res += vector->l0norm();}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {if (weight) res += featureGenerator->l0norm(inputs);}

  size_t res;
};

class ComputeEntropyFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  ComputeEntropyFeatureGeneratorCallback() : res(0) {}

  virtual void sense(size_t index, double value)
    {if (value > 10e-9) res -= value * log2(value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {if (weight) res += vector->entropy() * weight;}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {if (weight) res += featureGenerator->entropy(inputs) * weight;}

  double res;
};

class ComputeSumOfSquaresFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  ComputeSumOfSquaresFeatureGeneratorCallback() : res(0.0) {}

  virtual void sense(size_t index, double value)
    {res += value * value;}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {res += vector->sumOfSquares() * weight;}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {res += featureGenerator->sumOfSquares(inputs) * weight;}

  double res;
};

class ComputeMaximumValueFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  ComputeMaximumValueFeatureGeneratorCallback(size_t* index)
    : res(-DBL_MAX), index(index) {}

  virtual void sense(size_t index, double value)
    {add(index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
  {
    size_t best;
    double value = vector->getMaximumValue(&best) * weight;
    add(best, value);
  }

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
  {
    size_t best;
    double value = featureGenerator->getMaximumValue(inputs, &best) * weight;
    add(best, value);
  }

  void add(size_t index, double value)
  {
    if (value > res)
    {
      res = value;
      if (this->index)
        *this->index = index;
    }
  }

  double res;
  size_t* index;
};

class AppendToFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  AppendToFeatureGeneratorCallback(const SparseDoubleVectorPtr& target, size_t offsetInSparseVector)
    : target(target), offset(offsetInSparseVector) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(offset + index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {jassert(weight == 1.0); target->appendTo(target, offset + index);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {jassert(weight == 1.0); featureGenerator->appendTo(inputs, target, offset + index);}

protected:
  SparseDoubleVectorPtr target;
  size_t offset;
};

class AddWeightedToFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  AddWeightedToFeatureGeneratorCallback(const DenseDoubleVectorPtr& target, size_t offsetInSparseVector, double weight)
    : target(target), offset(offsetInSparseVector), weight(weight) {}

  virtual void sense(size_t index, double value)
    {target->getValueReference(index + offset) += value * weight;}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {vector->addWeightedTo(target, index + offset, weight * this->weight);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {featureGenerator->addWeightedTo(inputs, target, offset + index, weight * this->weight);}

protected:
  DenseDoubleVectorPtr target;
  size_t offset;
  double weight;
};

class DotProductFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  DotProductFeatureGeneratorCallback(const DenseDoubleVectorPtr& target, size_t offsetInSparseVector)
    : res(0.0), target(target), offset(offsetInSparseVector) {}

  virtual void sense(size_t index, double value)
    {res += target->getValue(index + offset) * value;}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {res += weight * vector->dotProduct(target, index + offset);}
 
  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {res += weight * featureGenerator->dotProduct(inputs, target, index + offset);}
 
  double res;

protected:
  DenseDoubleVectorPtr target;
  size_t offset;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_CALLBACKS_HPP_H_
