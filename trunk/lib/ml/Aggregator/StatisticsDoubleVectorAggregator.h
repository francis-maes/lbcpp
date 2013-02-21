/*------------------------------------------------.---------------------------------------.
 | Filename: StatisticsDoubleVectorAggregator.h   | Statistics Double Vector Aggregator   |
 | Author  : Denny Verbeeck                       |                                       |
 | Started : 20/02/2013 14:02                     |                                       |
 `------------------------------------------------/                                       |
 |                                                                                        |
 `---------------------------------------------------------------------------------------*/

#ifndef ML_AGGREGATOR_STATISTICS_DOUBLE_VECTOR_H_
# define ML_AGGREGATOR_STATISTICS_DOUBLE_VECTOR_H_

# include <ml/Aggregator.h>
# include <ml/Expression.h>
# include <ml/DoubleVector.h>

namespace lbcpp
{

class StatisticsDoubleVectorAggregator : public Aggregator
{
public:
  virtual bool doAcceptInputType(const ClassPtr& type) const
    {return type->inheritsFrom(denseDoubleVectorClass());}
  
  virtual ClassPtr initialize(const ClassPtr& inputsType)
    {return vectorClass(scalarVariableStatisticsClass);}
  
  virtual ObjectPtr compute(ExecutionContext& context, const std::vector<ObjectPtr>& inputs, ClassPtr outputType) const
  {
    if (inputs.empty())
      return ObjectPtr();
    
    size_t numValues = inputs[0].staticCast<DenseDoubleVector>()->getNumValues();
    
    // outputType or scalarVariableStatisticsClass?
    OVectorPtr res = new OVector(scalarVariableStatisticsClass, numValues);
    for (size_t i = 0; i < numValues; ++i)
      res->set(i, new ScalarVariableStatistics());
    
    for (size_t i = 0; i < inputs.size(); ++i)
    {
      DenseDoubleVectorPtr elem = inputs[i].staticCast<DenseDoubleVector>();
      jassert(elem->getNumValues() == numValues);
      for (size_t j = 0; j < numValues; ++j)
        res->getAndCast<ScalarVariableStatistics>(j)->push(elem->getValue(j));
    }
    return res;
  }
  
  struct AccumulatorData : public Object
  {
    AccumulatorData(IndexSetPtr indices, EnumerationPtr elementsEnumeration)
      : indices(indices), elementsEnumeration(elementsEnumeration), v(elementsEnumeration->getNumElements() * indices->size())
    {
      for (size_t i = 0; i < elementsEnumeration->getNumElements() * indices->size(); ++i)
        v[i] = new ScalarVariableStatistics();
    }
    
    IndexSetPtr indices;
    EnumerationPtr elementsEnumeration;
    std::vector<ScalarVariableStatisticsPtr> v;
  };
  typedef ReferenceCountedObjectPtr<AccumulatorData> AccumulatorDataPtr;
  
  virtual ObjectPtr startAggregation(const IndexSetPtr& indices, ClassPtr inputsType, ClassPtr outputType) const
    {return new AccumulatorData(indices, DoubleVector::getElementsEnumeration(inputsType));}
  
  virtual void updateAggregation(const ObjectPtr& d, const DataVectorPtr& inputs) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    
    jassert(data->indices == inputs->getIndices());
    jassert(inputs->getElementsType()->inheritsFrom(denseDoubleVectorClass()));
    
    size_t numElements = data->elementsEnumeration->getNumElements();
    ScalarVariableStatisticsPtr* dest = &data->v[0];
    for (DataVector::const_iterator it = inputs->begin(); it != inputs->end(); ++it)
    {
      const DenseDoubleVectorPtr& input = it.getRawObject().staticCast<DenseDoubleVector>();
      if (input)
      {
        jassert(numElements == input->getNumValues());
        for (size_t j = 0; j < numElements; ++j)
        {
          double value = input->getValue(j);
          jassert(isNumberValid(value));
          (*dest++)->push(value);
        }
      }
      else
      {
        for (size_t j = 0; j < numElements; ++j)
          (*dest++)->push(0.0);
      }
    }
  }
  
  virtual DataVectorPtr finalizeAggregation(const ObjectPtr& d) const
  {
    const AccumulatorDataPtr& data = d.staticCast<AccumulatorData>();
    ClassPtr svClass = vectorClass(scalarVariableStatisticsClass);
    const OVectorPtr& res = new OVector(svClass, data->indices->size());

    size_t numElements = data->elementsEnumeration->getNumElements();
    ScalarVariableStatisticsPtr* ptr = &data->v[0];
    for (size_t i = 0; i < res->getNumElements(); ++i)
    {
      OVectorPtr svStats = new OVector(scalarVariableStatisticsClass, numElements);
      for (size_t j = 0; j < numElements; ++j)
        svStats->set(j, *ptr++);
      res->set(i, svStats);
    }
    return new DataVector(data->indices, res);
  }
};
  
}; /* namespace lbcpp */

#endif // !ML_AGGREGATOR_STATISTICS_DOUBLE_VECTOR_H_
