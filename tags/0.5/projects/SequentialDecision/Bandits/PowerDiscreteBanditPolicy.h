/*-----------------------------------------.---------------------------------.
| Filename: PowerDiscreteBanditPolicy.h    | Power policy                    |
| Author  : Francis Maes                   |                                 |
| Started : 24/05/2011 20:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_BANDITS_DISCRETE_POLICY_POWER_H_
# define LBCPP_BANDITS_DISCRETE_POLICY_POWER_H_

# include "DiscreteBanditPolicy.h"

namespace lbcpp
{

class PowerDiscreteBanditPolicy : public IndexBasedDiscreteBanditPolicy, public Parameterized
{
public:
  PowerDiscreteBanditPolicy(size_t maxPower, bool useSparseSampler) : maxPower(maxPower), useSparseSampler(useSparseSampler)
  {
    jassert(maxPower >= 1);
    parametersEnumeration = createParametersEnumeration();
    parameters = new DenseDoubleVector(parametersEnumeration, doubleType);
  }

  PowerDiscreteBanditPolicy() : maxPower(1) {}

  virtual ObjectPtr computeGeneratedObject(ExecutionContext& context, const String& variableName)
    {return createParametersEnumeration();}

  virtual SamplerPtr createParametersSampler() const
  {
    SamplerPtr scalarSampler = gaussianSampler(0.0, 1.0);
    if (useSparseSampler)
      scalarSampler = zeroOrScalarContinuousSampler(bernoulliSampler(0.5, 0.1, 0.9), scalarSampler);
    return independentDoubleVectorSampler(parameters->getElementsEnumeration(), scalarSampler);
  }

  virtual void setParameters(const Variable& parameters)
    {this->parameters = parameters.getObjectAndCast<DenseDoubleVector>();}

  virtual Variable getParameters() const
    {return parameters;}

  EnumerationPtr createParametersEnumeration()
  {
    DefaultEnumerationPtr parametersEnumeration = new DefaultEnumeration(T("parameters"));

    static const char* names[4] = {"sqrt(log T)", "1/sqrt(T_i)", "mean(r)", "stddev(r)"};

    for (size_t i = 0; i <= maxPower; ++i)
      for (size_t j = 0; j <= maxPower; ++j)
        for (size_t k = 0; k <= maxPower; ++k)
          for (size_t l = 0; l <= maxPower; ++l)
          {
            if (i + j + k + l == 0)
              continue; // skip unit
            String name;
            if (i)
              name += String(names[0]) + (i > 1 ? T("^") + String((int)i) : String::empty) + T(" ");
            if (j)
              name += String(names[1]) + (j > 1 ? T("^") + String((int)j) : String::empty) + T(" ");
            if (k)
              name += String(names[2]) + (k > 1 ? T("^") + String((int)k) : String::empty) + T(" ");
            if (l)
              name += String(names[3]) + (l > 1 ? T("^") + String((int)l) : String::empty) + T(" ");
            name = name.trimEnd();
            parametersEnumeration->addElement(defaultExecutionContext(), name);
          }
    return parametersEnumeration;
  }

  virtual double computeBanditScore(size_t banditNumber, size_t timeStep, const std::vector<BanditStatisticsPtr>& banditStatistics) const
  {
    const BanditStatisticsPtr& bandit = banditStatistics[banditNumber];
    double v1 = sqrt(log((double)timeStep));
    double v2 = 1.0 / sqrt((double)bandit->getPlayedCount());
    double v3 = bandit->getRewardMean();
    double v4 = bandit->getRewardStandardDeviation();

    double res = 0.0;
    const double* parameter = parameters->getValuePointer(0);
    for (size_t i = 0; i <= maxPower; ++i)
      for (size_t j = 0; j <= maxPower; ++j)
        for (size_t k = 0; k <= maxPower; ++k)
          for (size_t l = 0; l <= maxPower; ++l)
          {
            if (i + j + k + l == 0)
              continue; // skip unit
            res += (*parameter++) * fastPow(v1, i) * fastPow(v2, j) * fastPow(v3, k) * fastPow(v4, l);
          }
    return res;
  }

  static double fastPow(double value, size_t power)
  {
    if (power == 0)
      return 1.0;
    else if (power == 1)
      return value;
    else if (power == 2)
      return value * value;
    else if (power == 3)
      return fastPow(value, 2) * value;
    else
      return pow(value, (double)power);
  }

protected:
  friend class PowerDiscreteBanditPolicyClass;

  size_t maxPower;
  bool useSparseSampler;
  EnumerationPtr parametersEnumeration;
  DenseDoubleVectorPtr parameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_BANDITS_DISCRETE_POLICY_POWER_H_
