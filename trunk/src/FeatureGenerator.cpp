/*-----------------------------------------.---------------------------------.
| Filename: FeatureGenerator.cpp           | Feature Generators              |
| Author  : Francis Maes                   |                                 |
| Started : 10/03/2009 18:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/FeatureGenerator/EditableFeatureGenerator.h>
#include <lbcpp/Object/ObjectGraph.h>
#include <lbcpp/Object/Table.h>
#include <lbcpp/impl/Bridge/FeatureGeneratorDefaultImplementations.hpp>
#include <lbcpp/impl/Bridge/DoubleVector.hpp>

#include "FeatureGenerator/EmptyFeatureGenerator.h"
#include "FeatureGenerator/UnitFeatureGenerator.h"
#include "FeatureGenerator/SubFeatureGenerator.h"
#include "FeatureGenerator/WeightedFeatureGenerator.h"
#include "FeatureGenerator/LinearCombinationFeatureGenerator.h"
#include "FeatureGenerator/ExplicitLinearCombinationFeatureGenerator.h"
using namespace lbcpp;

/*
** EditableFeatureGenerator
*/
void EditableFeatureGenerator::setDictionary(FeatureDictionaryPtr dictionary)
  {this->dictionary = dictionary;}

void EditableFeatureGenerator::ensureDictionary(FeatureDictionaryPtr dictionary)
{
  if (!dictionary)
    return;
  if (this->dictionary)
    checkDictionaryEquals(dictionary);
  else
    this->dictionary = dictionary;
}

FeatureDictionaryPtr EditableFeatureGenerator::getDictionary() const
{
  if (!dictionary)
    Object::error(getClassName() + "::getDictionary", "Missing dictionary");
  jassert(dictionary);
  return dictionary;
}

/*
** CompositeFeatureGenerator
*/
void CompositeFeatureGenerator::setSubGenerator(size_t index, FeatureGeneratorPtr featureGenerator)
{
  if (featureGenerators.size() < index + 1)
    featureGenerators.resize(index + 1, FeatureGeneratorPtr());
  featureGenerators[index] = featureGenerator;
  getDictionary()->ensureSubDictionary(index, featureGenerator->getDictionary());
}

void CompositeFeatureGenerator::appendSubGenerator(FeatureGeneratorPtr featureGenerator)
  {featureGenerators.push_back(featureGenerator);}

void CompositeFeatureGenerator::clear()
  {featureGenerators.clear();}

size_t CompositeFeatureGenerator::getNumSubGenerators() const
  {return featureGenerators.size();}
  
FeatureGeneratorPtr CompositeFeatureGenerator::getSubGenerator(size_t num) const
  {return getSubGeneratorWithIndex(num);}

size_t CompositeFeatureGenerator::getSubGeneratorIndex(size_t num) const
  {jassert(num < featureGenerators.size()); return num;}

FeatureGeneratorPtr CompositeFeatureGenerator::getSubGeneratorWithIndex(size_t index) const
  {jassert(index < featureGenerators.size()); return featureGenerators[index];}
    
/*
** LazyFeatureVector
*/
size_t LazyFeatureVector::getNumSubGenerators() const
  {return getResult()->getNumSubGenerators();}
  
FeatureGeneratorPtr LazyFeatureVector::getSubGenerator(size_t num) const
  {return getResult()->getSubGenerator(num);}

size_t LazyFeatureVector::getSubGeneratorIndex(size_t num) const
  {return getResult()->getSubGeneratorIndex(num);}

FeatureGeneratorPtr LazyFeatureVector::getSubGeneratorWithIndex(size_t index) const
  {return getResult()->getSubGeneratorWithIndex(index);}

FeatureVectorPtr LazyFeatureVector::getResult() const
{
  if (!result)
    const_cast<LazyFeatureVector* >(this)->result = computeVector();
  return result;
}

/*
** Lazy vectors
*/    
FeatureGeneratorPtr lbcpp::emptyFeatureGenerator()
{
  static FeatureGeneratorPtr instance = new EmptyFeatureGenerator();
  return instance;
}
FeatureGeneratorPtr lbcpp::unitFeatureGenerator()
{
  static FeatureGeneratorPtr instance = new UnitFeatureGenerator();
  return instance;
}

FeatureGeneratorPtr lbcpp::subFeatureGenerator(FeatureDictionaryPtr dictionary, size_t index, FeatureGeneratorPtr featureGenerator)
{
  return new SubFeatureGenerator(dictionary, index, featureGenerator);
}

FeatureGeneratorPtr lbcpp::multiplyByScalar(FeatureGeneratorPtr featureGenerator, double weight)
{
  jassert(featureGenerator);
  
  // x * 0 = <empty>
  if (weight == 0)
    return emptyFeatureGenerator();
  
  // x * 1 = x
  if (weight == 1)
    return featureGenerator;
    
  if (featureGenerator.isInstanceOf<EditableFeatureGenerator>())
  {
    // (k1 * x) * k2 = ((k1 * k2) * x)
    WeightedFeatureGeneratorPtr weighted = featureGenerator.dynamicCast<WeightedFeatureGenerator>();
    if (weighted)
      return weighted->exists()
        ? multiplyByScalar(weighted->getFeatureGenerator(), weight * weighted->getWeight())
        : emptyFeatureGenerator();
      
    // k * (sum_i w_i * x_i) = sum_i (w_i * k) * x_i
    LinearCombinationFeatureGeneratorPtr combination = featureGenerator.dynamicCast<LinearCombinationFeatureGenerator>();
    if (combination)
    {
      if (combination->exists())
        return linearCombination(combination->getCompositeFeatureGenerator(),
                  lbcpp::multiplyByScalar(combination->getWeights(), weight)->toDenseVector());
      else
        return emptyFeatureGenerator();
    }
    
    // k * (composite(x_1, ..., x_n)) = composite(k * x_1, ... k * x_n)
    CompositeFeatureGeneratorPtr composite = featureGenerator.dynamicCast<CompositeFeatureGenerator>();
    if (composite)
    {
      size_t n = composite->getNumSubGenerators();
      if (n)
      {
        CompositeFeatureGeneratorPtr res = new CompositeFeatureGenerator(featureGenerator->getDictionary(), n);
        for (size_t i = 0; i < n; ++i)
          res->setSubGenerator(composite->getSubGeneratorIndex(i), multiplyByScalar(composite->getSubGenerator(i), weight));
        return res;
      }
      else
        return emptyFeatureGenerator();
    }
    
    // k * sub(index, x) = sub(index, k * x)
    SubFeatureGeneratorPtr sub = featureGenerator.dynamicCast<SubFeatureGenerator>();
    if (sub)
      return sub->exists()
        ? subFeatureGenerator(sub->getDictionary(), sub->getIndex(), multiplyByScalar(sub->getFeatureGenerator(), weight))
        : emptyFeatureGenerator();
  }

  // k * <empty> = <empty>
  if (featureGenerator.dynamicCast<EmptyFeatureGenerator>())
    return emptyFeatureGenerator();

  // default: k * x 
  return new WeightedFeatureGenerator(featureGenerator, weight);
}

FeatureGeneratorPtr lbcpp::weightedSum(FeatureGeneratorPtr featureGenerator1, double weight1, FeatureGeneratorPtr featureGenerator2, double weight2, bool computeNow)
{
  if (computeNow)
  {
    if (featureGenerator1->isDense() || featureGenerator2->isDense())
    {
      DenseVectorPtr res = new DenseVector(featureGenerator1->getDictionary());
      res->addWeighted(featureGenerator1, weight1);
      res->addWeighted(featureGenerator2, weight2);
      return res;
    }
    else
    {
      SparseVectorPtr res = new SparseVector(featureGenerator1->getDictionary());
      res->addWeighted(featureGenerator1, weight1);
      res->addWeighted(featureGenerator2, weight2);
      return res;
    }
  }
  else
  {
    std::vector<std::pair<FeatureGeneratorPtr, double> >* combination =
      new std::vector<std::pair<FeatureGeneratorPtr, double> >(2);
    (*combination)[0] = std::make_pair(featureGenerator1, weight1);
    (*combination)[1] = std::make_pair(featureGenerator2, weight2);
    return linearCombination(combination);
  }
}

FeatureGeneratorPtr lbcpp::linearCombination(FeatureGeneratorPtr compositeFeatureGenerator, DenseVectorPtr weights)
{
  return new LinearCombinationFeatureGenerator(compositeFeatureGenerator, weights);
}

FeatureGeneratorPtr lbcpp::linearCombination(std::vector< std::pair<FeatureGeneratorPtr, double> >* newTerms)
{
  return new ExplicitLinearCombinationFeatureGenerator(newTerms);
}

class FeatureGeneratorTable : public Table
{
public:
  FeatureGeneratorTable(FeatureGeneratorPtr featureGenerator)
    : header(new TableHeader())
  {
    header->addColumn("name", TableHeader::stringType);
    header->addColumn("value", TableHeader::doubleType);
    header->addColumn("absvalue", TableHeader::doubleType);
    featureGenerator->accept(new EnumerateFeatures(features));
  }
  
  virtual TableHeaderPtr getHeader() const
    {return header;}
    
  virtual size_t getNumRows() const
    {return features.size();}
  
  virtual double getDouble(size_t rowNumber, size_t columnNumber) const
  {
    jassert(columnNumber == 1 || columnNumber == 2);
    double value = features[rowNumber].second;
    return columnNumber == 1 || value >= 0 ? value : -value;
  }
  
  virtual String getString(size_t rowNumber, size_t columnNumber) const
  {
    jassert(columnNumber == 0);
    return features[rowNumber].first;
  }

private:
  TableHeaderPtr header;
  std::vector< std::pair<String, double> > features;
  
  struct EnumerateFeatures : public PathBasedFeatureVisitor
  {
    EnumerateFeatures(std::vector< std::pair<String, double> >& features)
      : features(features) {}
      
    std::vector< std::pair<String, double> >& features;
    
    virtual void featureSense(const std::vector<size_t>& path, const String& name, double value)
      {features.push_back(std::make_pair(name, value));}
  };
};

TablePtr FeatureGenerator::toTable() const
  {return new FeatureGeneratorTable(const_cast<FeatureGenerator* >(this));}

class FeatureGeneratorGraph : public ObjectGraph
{
public:
  FeatureGeneratorGraph(FeatureGeneratorPtr generator)
    : roots(1, generator) {}
  FeatureGeneratorGraph() {}
  
  void addFeatureGenerator(FeatureGeneratorPtr featureGenerator)
    {roots.push_back(featureGenerator);}
  
  virtual size_t getNumRoots() const
    {return roots.size();}
    
  virtual ObjectPtr getRoot(size_t index) const
    {jassert(index < roots.size()); return roots[index];}

  virtual void setRoots(const std::vector<ObjectPtr>& successors)
    {roots = *(const std::vector<FeatureGeneratorPtr>* )(&successors);}
  
  virtual void setSuccessors(ObjectPtr node, const std::vector<ObjectPtr>& successors)
    {jassert(false);} // not implemented
    
  virtual size_t getNumSuccessors(ObjectPtr node) const
  {
    FeatureGeneratorPtr featureGenerator = node.dynamicCast<FeatureGenerator>();
    if (featureGenerator)
      return 1 + featureGenerator->getNumSubGenerators();
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    jassert(dictionary);
    return dictionary->getNumSubDictionaries();
  }
  
  virtual ObjectPtr getSuccessor(ObjectPtr node, size_t index) const
  {
    FeatureGeneratorPtr featureGenerator = node.dynamicCast<FeatureGenerator>();
    if (featureGenerator)
    {
      if (index == 0)
        return featureGenerator->getDictionary();
      return featureGenerator->getSubGenerator(index - 1);
    }
    FeatureDictionaryPtr dictionary = node.dynamicCast<FeatureDictionary>();
    jassert(dictionary);
    return dictionary->getSubDictionary(index);
  }

  // serialisation is not implemented
  virtual void saveNode(OutputStream& ostr, const ObjectPtr node) const
    {jassert(false);}
  virtual ObjectPtr loadNode(InputStream& istr) const
    {jassert(false); return ObjectPtr();}
    
protected:
  std::vector<FeatureGeneratorPtr> roots;
};

ObjectGraphPtr FeatureGenerator::toGraph() const
  {return new FeatureGeneratorGraph(const_cast<FeatureGenerator* >(this));}

void FeatureGenerator::getChildrenObjects(std::vector< std::pair<String, ObjectPtr> >& res) const
{
  StringDictionaryPtr scopes = getDictionary()->getScopes();
  res.resize(getNumSubGenerators());
  for (size_t i = 0; i < res.size(); ++i)
  {
    res[i].first = scopes->getString(getSubGeneratorIndex(i));
    res[i].second = getSubGenerator(i);
  }
}
