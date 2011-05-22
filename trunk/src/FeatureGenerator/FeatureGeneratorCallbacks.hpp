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
    {vector->appendTo(target, index, weight);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {featureGenerator->appendTo(inputs, target, index, weight);}

private:
  SparseDoubleVectorPtr target;
};

class FillDenseVectorFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  FillDenseVectorFeatureGeneratorCallback(const DenseDoubleVectorPtr& target)
    : target(target) {}

  virtual void sense(size_t index, double value)
    {target->setValue(index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {vector->addWeightedTo(target, index, weight);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {featureGenerator->addWeightedTo(inputs, target, index, weight);}

private:
  DenseDoubleVectorPtr target;
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

class ComputeL1NormFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  ComputeL1NormFeatureGeneratorCallback() : res(0.0) {}

  virtual void sense(size_t index, double value)
    {res += fabs(value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {if (weight) res += fabs(weight) * vector->l1norm();}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {if (weight) res += fabs(weight) * featureGenerator->l1norm(inputs);}

  double res;
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

class ComputeExtremumValueFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  ComputeExtremumValueFeatureGeneratorCallback(bool lookForMaximum, size_t* index)
    : res(lookForMaximum ? -DBL_MAX : DBL_MAX), lookForMaximum(lookForMaximum), index(index) {}

  virtual void sense(size_t index, double value)
    {add(index, value);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
  {
    size_t best;
    double value = vector->getExtremumValue(lookForMaximum, &best) * weight;
    add(best, value);
  }

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
  {
    size_t best;
    double value = featureGenerator->getExtremumValue(inputs, lookForMaximum, &best) * weight;
    add(best, value);
  }

  void add(size_t index, double value)
  {
    if ((lookForMaximum && value > res) ||
        (!lookForMaximum && value < res))
    {
      res = value;
      if (this->index)
        *this->index = index;
    }
  }

  double res;
  bool lookForMaximum;
  size_t* index;
};

class AppendToFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  AppendToFeatureGeneratorCallback(const SparseDoubleVectorPtr& target, size_t offsetInSparseVector, double weight)
    : target(target), offset(offsetInSparseVector), weight(weight) {}

  virtual void sense(size_t index, double value)
    {target->appendValue(offset + index, value * weight);}

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {vector->appendTo(target, offset + index, weight * this->weight);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {featureGenerator->appendTo(inputs, target, offset + index, weight * this->weight);}

protected:
  SparseDoubleVectorPtr target;
  size_t offset;
  double weight;
};

class AddWeightedToSparseFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  AddWeightedToSparseFeatureGeneratorCallback(const SparseDoubleVectorPtr& target, size_t offsetInSparseVector, double weight)
    : target(target), offset(offsetInSparseVector), weight(weight), lastIndex(-1) {}

  virtual void sense(size_t index, double value)
  {
    index += offset;
    jassert((int)index > lastIndex);
    if ((int)index > target->getLastIndex())
      target->appendValue(index, value * weight);
    else
      target->incrementValue(index, value * weight);
  }

  virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
    {vector->addWeightedTo(target, index + offset, weight * this->weight);}

  virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
    {featureGenerator->addWeightedTo(inputs, target, offset + index, weight * this->weight);}

protected:
  SparseDoubleVectorPtr target;
  size_t offset;
  double weight;
  int lastIndex;
};

class AddWeightedToDenseFeatureGeneratorCallback : public FeatureGeneratorCallback
{
public:
  AddWeightedToDenseFeatureGeneratorCallback(const DenseDoubleVectorPtr& target, size_t offsetInDenseVector, double weight)
    : target(target), offset(offsetInDenseVector), weight(weight) {}

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
