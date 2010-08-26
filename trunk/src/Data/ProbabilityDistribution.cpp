/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistribution.cpp    | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/ProbabilityDistribution.h>
#include <lbcpp/Data/RandomGenerator.h>
#include <lbcpp/Data/XmlSerialisation.h>
using namespace lbcpp;

/*
** BernoulliDistribution
*/
double BernoulliDistribution::compute(const Variable& value) const
{
  if (!value.isNil() && checkInheritance(value, booleanType()))
    return value.getBoolean() ? getProbabilityOfTrue() : getProbabilityOfFalse();
  return 0.0;
}

Variable BernoulliDistribution::sample(RandomGeneratorPtr random) const
  {return random->sampleBool(getProbabilityOfTrue());}
  
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

/*
** DiscreteProbabilityDistribution
*/
DiscreteProbabilityDistribution::DiscreteProbabilityDistribution(EnumerationPtr enumeration)
  : ProbabilityDistribution(discreteProbabilityDistributionClass(enumeration)), values(enumeration->getNumElements() + 1, 0.0), sum(0.0) {}

String DiscreteProbabilityDistribution::toString() const
{
  EnumerationPtr enumeration = getEnumeration();
  String str;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i])
    {
      if (str.isNotEmpty())
        str += ' ';
      if (i == values.size() - 1)
        str += '_';
      else if (enumeration->hasOneLetterCodes())
        str += enumeration->getOneLetterCode(i);
      else
        str += enumeration->getElementName(i);
      str += ' ';
      str += String(values[i]);
    }
  return str;
}

Variable DiscreteProbabilityDistribution::sample(RandomGeneratorPtr random) const
{
  EnumerationPtr enumeration = getEnumeration();
  int res = random->sampleWithProbabilities(values, sum);
  if (res >= 0 && res < (int)enumeration->getNumElements())
    return Variable(res, enumeration);
  else
    return Variable();
}

double DiscreteProbabilityDistribution::compute(const Variable& value) const
{
  if (value.isNil())
    return sum ? values.back() / sum : 0.0;
  EnumerationPtr enumeration = getEnumeration();
  if (!checkInheritance(value, enumeration))
    return 0.0;
  int index = value.getInteger();
  jassert(index >= 0 && index < (int)enumeration->getNumElements());
  return sum ? values[index] / sum : 0.0;
}

double DiscreteProbabilityDistribution::computeEntropy() const
{
  if (!sum)
    return 0.0;
  double res = 0.0;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i])
    {
      double p = values[i] / sum;
      res -= p * log2(p);
    }
  return res;
}

void DiscreteProbabilityDistribution::increment(const Variable& value)
{
  if (value.isNil())
  {
    ++values.back();
    ++sum;
  }
  else if (checkInheritance(value, getEnumeration()))
  {
    ++(values[value.getInteger()]);
    ++sum;
  }
}

ObjectPtr DiscreteProbabilityDistribution::multiplyByScalar(double scalar)
{
  sum = 0.0;
  for (size_t i = 0; i < values.size(); ++i)
  {
    values[i] *= scalar;
    sum += values[i];
  }
  return ObjectPtr(this);
}

ObjectPtr DiscreteProbabilityDistribution::addWeighted(const Variable& value, double weight)
{
  if (value.isNil())
  {
    values.back() += weight;
    sum += weight;
  }
  if (value.getType() == getEnumeration())
  {
    values[(size_t)value.getInteger()] += weight;
    sum += weight;
  }
  else if (value.getType() == getClass())
  {
    DiscreteProbabilityDistributionPtr other = value.getObjectAndCast<DiscreteProbabilityDistribution>();
    jassert(values.size() == other->values.size());
    for (size_t i = 0; i < values.size(); ++i)
      values[i] += other->values[i] * weight;
    sum += other->sum * weight;
  }
  else
    jassert(false);
  return ObjectPtr(this);
}

void DiscreteProbabilityDistribution::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

bool DiscreteProbabilityDistribution::loadFromString(const String& str, MessageCallback& callback)
{
  EnumerationPtr enumeration = getEnumeration();
  jassert(enumeration);
  values.resize(enumeration->getNumElements() + 1, 0.0);
  sum = 0.0;
  
  StringArray tokens;
  tokens.addTokens(str, true);
  if (tokens.size() % 2 == 1)
  {
    callback.errorMessage(T("DiscreteProbabilityDistribution::loadFromString"), T("Invalid number of arguments in ") + str.quoted());
    return false;
  }
  
  for (int i = 0; i < tokens.size() - 1; i += 2)
  {
    String indexString = tokens[i];
    double value = tokens[i+1].getDoubleValue();
    int index;
    if (enumeration->hasOneLetterCodes() && indexString.length() == 1)
    {
      if (indexString[0] == '_')
        index = (int)enumeration->getNumElements();
      else
      {
        index = enumeration->getOneLetterCodes().indexOfChar(indexString[0]);
        if (index < 0)
        {
          callback.errorMessage(T("DiscreteProbabilityDistribution::loadFromString"), T("Could not find one-letter code ") + indexString.quoted());
          return false;
        }
      }
    }
    else
    {
      index = enumeration->findElement(indexString);
      if (index < 0)
      {
        callback.errorMessage(T("DiscreteProbabilityDistribution::loadFromString"), T("Could not find element ") + indexString.quoted());
        return false;
      }
    }
    jassert(index >= 0 && index < (int)values.size());
    values[index] = value;
    sum += value;
  }
  return true;
}

bool DiscreteProbabilityDistribution::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getAllSubText(), importer.getCallback());}
