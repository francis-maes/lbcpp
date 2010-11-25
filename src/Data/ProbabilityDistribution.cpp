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
  if (!value.isNil() && checkInheritance(value, booleanType))
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
  : ProbabilityDistribution(discreteProbabilityDistributionClass(enumeration)), values(enumeration->getNumElements() + 1, 0.0), count(0) {}

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
  int res = (int)random->sampleWithProbabilities(values);
  if (res >= 0 && res < (int)enumeration->getNumElements())
    return Variable(res, enumeration);
  else
    return Variable();
}

double DiscreteProbabilityDistribution::compute(const Variable& value) const
{
  if (value.isNil())
    return values.back();
  if (!checkInheritance(value, getEnumeration()))
    return 0.0;
  int index = value.getInteger();
  jassert(index >= 0 && index < (int)values.size());
  return values[index];
}

double DiscreteProbabilityDistribution::computeEntropy() const
{
  ScopedLock _(cachedEntropyLock);
  if (cachedEntropy.exists())
    return cachedEntropy.getDouble();
  double res = 0.0;
  for (size_t i = 0; i < values.size(); ++i)
    if (values[i])
    {
      double p = values[i];
      res -= p * log2(p);
    }
  const_cast<DiscreteProbabilityDistribution* >(this)->cachedEntropy = Variable(res);
  return res;
}

void DiscreteProbabilityDistribution::increment(const Variable& value)
{
  size_t index;
  if (value.isNil())
    index = values.size() - 1;
  else if (checkInheritance(value, getEnumeration()))
    index = (size_t)value.getInteger();
  else
    return;
  ++count;
  setProbability(index, getProbability(index) + 1.0);
}

void DiscreteProbabilityDistribution::normalize()
{
  if (count <= 1)
    return;
  for (size_t i = 0; i < values.size(); ++i)
    setProbability(i, getProbability(i) / (double)count);
  count = 0;
}

void DiscreteProbabilityDistribution::saveToXml(XmlExporter& exporter) const
  {exporter.addTextElement(toString());}

bool DiscreteProbabilityDistribution::loadFromString(ExecutionContext& context, const String& str)
{
  EnumerationPtr enumeration = getEnumeration();
  jassert(enumeration);
  values.resize(enumeration->getNumElements() + 1, 0.0);
  
  StringArray tokens;
  tokens.addTokens(str, true);
  if (tokens.size() % 2 == 1)
  {
    context.errorCallback(T("DiscreteProbabilityDistribution::loadFromString"), T("Invalid number of arguments in ") + str.quoted());
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
          context.errorCallback(T("DiscreteProbabilityDistribution::loadFromString"), T("Could not find one-letter code ") + indexString.quoted());
          return false;
        }
      }
    }
    else
    {
      index = enumeration->findElement(indexString);
      if (index < 0)
      {
        context.errorCallback(T("DiscreteProbabilityDistribution::loadFromString"), T("Could not find element ") + indexString.quoted());
        return false;
      }
    }
    setProbability((size_t)index, value);
  }
  return true;
}

bool DiscreteProbabilityDistribution::loadFromXml(XmlImporter& importer)
  {return loadFromString(importer.getContext(), importer.getAllSubText());}

void DiscreteProbabilityDistribution::setProbability(size_t index, double probability)
{
  jassert(index < values.size());
  values[index] = probability;

  ScopedLock _(cachedEntropyLock);
  cachedEntropy.clear();
}
