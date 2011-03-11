/*-----------------------------------------.---------------------------------.
| Filename: DiscreteProbabilityDistrib..cpp| Discrete Probability            |
| Author  : Julien Becker                  | Distributions                   |
| Started : 26/11/2010 11:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Data/RandomGenerator.h>
#include <lbcpp/Distribution/DiscreteDistribution.h>
#include "Builder/BernoulliDistributionBuilder.h"
#include "Builder/EnumerationDistributionBuilder.h"
#include "Builder/GaussianDistributionBuilder.h"

using namespace lbcpp;

// todo: ranger
TypePtr Distribution::getTemplateParameter(TypePtr type)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Distribution"));
  jassert(dvType && dvType->getNumTemplateArguments() == 1);
  TypePtr res = dvType->getTemplateArgument(0);
  jassert(res);
  return res;
}

bool Distribution::getTemplateParameter(ExecutionContext& context, TypePtr type, TypePtr& res)
{
  TypePtr dvType = type->findBaseTypeFromTemplateName(T("Distribution"));
  if (!dvType)
  {
    context.errorCallback(type->getName() + T(" is not a Distribution"));
    return false;
  }
  jassert(dvType->getNumTemplateArguments() == 1);
  res = dvType->getTemplateArgument(0);
  return true;
}


/*
 ** BernoulliDistribution
 */
double BernoulliDistribution::computeProbability(const Variable& value) const
{
  if (!value.isNil() && checkInheritance(value, booleanType))
    return value.getBoolean() ? getProbabilityOfTrue() : getProbabilityOfFalse();
  return 0.0;
}

Variable BernoulliDistribution::sample(RandomGeneratorPtr random) const
  {return random->sampleBool(getProbabilityOfTrue());}

Variable BernoulliDistribution::sampleBest(RandomGeneratorPtr random) const
{
  double p = getProbabilityOfFalse();
  bool res;
  if (p > 0.5)
    res = true;
  else if (p < 0.5)
    res = false;
  else
    res = random->sampleBool();
  return res;
}

double BernoulliDistribution::computeEntropy() const
{
  double p = getProbabilityOfTrue();
  double res = 0.0;
  if (p)
    res -= p * log2(p);
  p = 1.0 - p;
  if (p)
    res -= p * log2(p);
  return res;
}

DistributionBuilderPtr BernoulliDistribution::createBuilder() const
  {return new BernoulliDistributionBuilder();}

/*
 ** EnumerationDistribution
 */
EnumerationDistribution::EnumerationDistribution(EnumerationPtr enumeration)
  : DiscreteDistribution(enumerationDistributionClass(enumeration)), values(enumeration->getNumElements() + 1, 0.0) {}

EnumerationDistribution::EnumerationDistribution(EnumerationPtr enumeration, const std::vector<double>& probabilities)
  : DiscreteDistribution(enumerationDistributionClass(enumeration)), values(probabilities)
  {jassert(probabilities.size() == enumeration->getNumElements() + 1);}

String EnumerationDistribution::toString() const
{
  EnumerationPtr enumeration = getEnumeration();
  String str;
  bool hasOneLetterCodes = enumeration->hasOneLetterCodes();
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i])
    {
      if (str.isNotEmpty())
        str += ' ';
      if (i == values.size() - 1)
        str += '_';
      else
      {
        EnumerationElementPtr element = enumeration->getElement(i);
        if (hasOneLetterCodes)
          str += element->getOneLetterCode();
        else
          str += element->getName();
      }
      str += ' ';
      str += String(values[i]);
    }
  return str;
}

Variable EnumerationDistribution::sample(RandomGeneratorPtr random) const
{
  EnumerationPtr enumeration = getEnumeration();
  int res = (int)random->sampleWithProbabilities(values);
  if (res >= 0 && res < (int)enumeration->getNumElements())
    return Variable(res, enumeration);
  else
    return Variable();
}

Variable EnumerationDistribution::sampleBest(RandomGeneratorPtr random) const
{
  EnumerationPtr enumeration = getEnumeration();
  std::set<size_t> bestIndices;
  double bestValue = -DBL_MAX;

  for (size_t i = 0; i < values.size(); ++i)
    if (values[i] >= bestValue)
    {
      if (values[i] != bestValue)
      {
        bestValue = values[i];
        bestIndices.clear();
      }
      bestIndices.insert(i);
    }

  if (!bestIndices.size())
    return Variable::missingValue(enumeration);

  size_t s = random->sampleSize(bestIndices.size());
  std::set<size_t>::iterator it = bestIndices.begin();
  for (size_t i = 0; i < s; ++i)
  {
    ++it;
    jassert(it != bestIndices.end());
  }
  return Variable(*it, enumeration);
} 

double EnumerationDistribution::computeProbability(const Variable& value) const
{
  if (value.isNil())
    return values.back();
  if (!checkInheritance(value, getEnumeration()))
    return 0.0;
  int index = value.getInteger();
  jassert(index >= 0 && index < (int)values.size());
  return values[index];
}

double EnumerationDistribution::computeEntropy() const
{
  ScopedLock _(cachedEntropyLock);
  if (cachedEntropy.exists())
    return cachedEntropy.getDouble();
  // todo: use DenseDoubleVector::computeEntropy() and remove cachedEntropy
  double res = 0.0;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i] > 1e-9)
    {
      double p = values[i];
      res -= p * log2(p);
    }
  const_cast<EnumerationDistribution* >(this)->cachedEntropy = Variable(res);
  return res;
}

DistributionBuilderPtr EnumerationDistribution::createBuilder() const
  {return new EnumerationDistributionBuilder(getEnumeration());}

void EnumerationDistribution::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

bool EnumerationDistribution::loadFromString(ExecutionContext& context, const String& str)
{
  EnumerationPtr enumeration = getEnumeration();
  jassert(enumeration);
  values.resize(enumeration->getNumElements() + 1, 0.0);
  
  StringArray tokens;
  tokens.addTokens(str, true);
  if (tokens.size() % 2 == 1)
  {
    context.errorCallback(T("DiscreteDistribution::loadFromString"), T("Invalid number of arguments in ") + str.quoted());
    return false;
  }
  
  bool hasOneLetterCodes = enumeration->hasOneLetterCodes();
  
  for (int i = 0; i < tokens.size() - 1; i += 2)
  {
    String indexString = tokens[i];
    double value = tokens[i+1].getDoubleValue();
    int index;
    if (hasOneLetterCodes && indexString.length() == 1)
    {
      if (indexString[0] == '_')
        index = (int)enumeration->getNumElements();
      else
      {
        index = enumeration->findElementByOneLetterCode(indexString[0]);
        if (index < 0)
        {
          context.errorCallback(T("DiscreteDistribution::loadFromString"), T("Could not find one-letter code ") + indexString.quoted());
          return false;
        }
      }
    }
    else
    {
      index = enumeration->findElementByName(indexString);
      if (index < 0)
      {
        context.errorCallback(T("DiscreteDistribution::loadFromString"), T("Could not find element ") + indexString.quoted());
        return false;
      }
    }
    values[index] = value;
  }
  return true;
}

bool EnumerationDistribution::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

/*
 ** IntegerGaussianDistribution
 */
DistributionBuilderPtr IntegerGaussianDistribution::createBuilder() const
  {return new IntegerGaussianDistributionBuilder();}

/*
 ** PositiveIntegerGaussianDistribution
 */
DistributionBuilderPtr PositiveIntegerGaussianDistribution::createBuilder() const
  {return new PositiveIntegerGaussianDistributionBuilder();}
