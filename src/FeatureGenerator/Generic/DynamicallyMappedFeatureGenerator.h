/*-----------------------------------------.---------------------------------.
| Filename: DynamicallyMappedFeatureGener.h| Dynamically Mapped              |
| Author  : Francis Maes                   |          Feature Generator      |
| Started : 22/03/2011 19:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_GENERATOR_GENERIC_DYNAMICALLY_MAPPED_H_
# define LBCPP_FEATURE_GENERATOR_GENERIC_DYNAMICALLY_MAPPED_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

class DynamicSubsetEnumeration : public Enumeration
{
public:
  DynamicSubsetEnumeration(EnumerationPtr targetEnumeration, size_t reservedSize)
    : Enumeration(T("subset(") + targetEnumeration->getName() + T(")")), targetEnumeration(targetEnumeration), reservedSize(reservedSize)
  {
  }

  virtual size_t getNumElements() const
    {return reservedSize;}

  virtual EnumerationElementPtr getElement(size_t index) const
  {
    if (index < mapping.size())
      return targetEnumeration->getElement(mapping[index]);
    else
      return EnumerationElementPtr(); // not yet defined
  }

  int getElementOrAddIfMissing(size_t targetIndex)
  {
    std::map<size_t, size_t>::const_iterator it = inverseMapping.find(targetIndex);
    if (it == inverseMapping.end())
      return addElement(targetIndex);
    else
      return (int)it->second;
  }

  int addElement(size_t targetIndex)
  {
    if (mapping.size() >= reservedSize)
      return -1; // sorry, not enough reserved elements
    size_t res = mapping.size();
    mapping.push_back(targetIndex);
    inverseMapping[targetIndex] = res;
    return res;
  }

protected:
  EnumerationPtr targetEnumeration;
  size_t reservedSize;
  std::vector<size_t> mapping;  // index -> targetIndex
  std::map<size_t, size_t> inverseMapping; // targetIndex -> index
};

typedef ReferenceCountedObjectPtr<DynamicSubsetEnumeration> DynamicSubsetEnumerationPtr;


// DoubleVector[n], DoubleVector[m] -> DoubleVector[n x m]
class DynamicallyMappedFeatureGenerator : public FeatureGenerator
{
public:
  DynamicallyMappedFeatureGenerator(FeatureGeneratorPtr baseFeatureGenerator, size_t reservedSize, bool lazy)
    : FeatureGenerator(lazy), reservedSize(reservedSize), baseFeatureGenerator(baseFeatureGenerator) {}

  DynamicallyMappedFeatureGenerator() {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return baseFeatureGenerator->getMinimumNumRequiredInputs();}

  virtual size_t getMaximumNumRequiredInputs() const
    {return baseFeatureGenerator->getMaximumNumRequiredInputs();}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return baseFeatureGenerator->getRequiredInputType(index, numInputs);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    if (!baseFeatureGenerator->initialize(context, inputVariables))
      return false;

    elementsType = baseFeatureGenerator->getFeaturesType();
    outputName = baseFeatureGenerator->getOutputVariable()->getName();
    outputShortName = baseFeatureGenerator->getOutputVariable()->getShortName();
    EnumerationPtr enumeration = baseFeatureGenerator->getFeaturesEnumeration();
    return (features = new DynamicSubsetEnumeration(enumeration, reservedSize));
  }

  struct Callback : public FeatureGeneratorCallback
  {
    Callback(const DynamicSubsetEnumerationPtr& features)
      : features(features) {}

    int getFeatureIndex(size_t conjunctionIndex) const
      {ScopedLock _(featuresLock); return features->getElementOrAddIfMissing(conjunctionIndex);}

    virtual void sense(size_t index, double value)
    {
      int featureIndex = getFeatureIndex(index);
      if (featureIndex >= 0)
        values[featureIndex] = value;
    }

    virtual void sense(size_t index, const DoubleVectorPtr& vector, double weight)
      {jassert(false);} // not supported yet

    virtual void sense(size_t index, const FeatureGeneratorPtr& featureGenerator, const Variable* inputs, double weight)
      {jassert(false);} // not supported yet

    const std::map<size_t, double>& getValues() const
      {return values;}

  private:
    CriticalSection featuresLock;
    DynamicSubsetEnumerationPtr features;
    std::map<size_t, double> values;
  };

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    Callback c(features);
    baseFeatureGenerator->computeFeatures(inputs, c);
    const std::map<size_t, double>& values = c.getValues();
    for (std::map<size_t, double>::const_iterator it = values.begin(); it != values.end(); ++it)
      callback.sense(it->first, it->second);
  }
  
protected:
  friend class DynamicallyMappedFeatureGeneratorClass;

  size_t reservedSize;
  FeatureGeneratorPtr baseFeatureGenerator;
  DynamicSubsetEnumerationPtr features;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_GENERATOR_GENERIC_DYNAMICALLY_MAPPED_H_
