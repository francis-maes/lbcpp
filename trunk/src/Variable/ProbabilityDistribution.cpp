/*-----------------------------------------.---------------------------------.
| Filename: ProbabilityDistribution.cpp    | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/ProbabilityDistribution.h>
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

Variable BernoulliDistribution::sample(RandomGenerator& random) const
  {return random.sampleBool(getProbabilityOfTrue());}
  
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
  : enumeration(enumeration), values(enumeration->getNumElements() + 1, 0.0), sum(0.0) {}

TypePtr DiscreteProbabilityDistribution::getClass() const
  {return discreteProbabilityDistributionClass(enumeration);}

String DiscreteProbabilityDistribution::toString() const
  {return T("[") + variablesToString(T(", ")) + T("]");}

Variable DiscreteProbabilityDistribution::sample(RandomGenerator& random) const
{
  int res = random.sampleWithProbabilities(values, sum);
  if (res >= 0 && res < (int)enumeration->getNumElements())
    return Variable(res, enumeration);
  else
    return Variable();
}

double DiscreteProbabilityDistribution::compute(const Variable& value) const
{
  if (value.isNil())
    return sum ? values.back() / sum : 0.0;
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

void DiscreteProbabilityDistribution::setVariable(size_t index, const Variable& value)
{
  jassert(value.isDouble());
  sum -= values[index];
  values[index] = value.getDouble();
  sum += values[index];
}

Variable DiscreteProbabilityDistribution::getVariable(size_t index) const
  {jassert(index < values.size()); return Variable(values[index], probabilityType());}

void DiscreteProbabilityDistribution::increment(const Variable& value)
{
  if (value.isNil())
  {
    ++values.back();
    ++sum;
  }
  else if (checkInheritance(value, enumeration))
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
  if (value.getType() == enumeration)
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

ClassPtr lbcpp::discreteProbabilityDistributionClass(EnumerationPtr enumeration)
{
  static UnaryTemplateTypeCache cache(T("DiscreteProbabilityDistribution"));
  return cache(enumeration);
}

void declareProbabilityDistributionClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(ProbabilityDistribution, Object);
    LBCPP_DECLARE_TEMPLATE_CLASS(DiscreteProbabilityDistribution, 1, ProbabilityDistribution);
}
