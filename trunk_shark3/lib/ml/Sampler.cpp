/*-----------------------------------------.---------------------------------.
| Filename: Sampler.cpp                    | Sampler                         |
| Author  : Francis Maes                   |  (represents a distribution)    |
| Started : 07/03/2013 17:44               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <ml/Sampler.h>
#include <boost/math/special_functions/erf.hpp>
#include <shark/SharkDefs.h>
using namespace lbcpp;

/*
** GaussianSampler
*/
ObjectPtr GaussianSampler::sample(ExecutionContext& context) const
  {return new Double(context.getRandomGenerator()->sampleDoubleFromGaussian(mean, standardDeviation));}
  
bool GaussianSampler::isDeterministic() const
  {return standardDeviation < 1e-12;}
  
double GaussianSampler::probabilityDensityFunction(double x, double mu, double sigma)
{
  return exp(-1 * (x - mu) * (x - mu) / (2 * sigma * sigma)) / (sigma * sqrt(M_2_TIMES_PI));
}

double GaussianSampler::cumulativeDensityFunction(double x, double mu, double sigma)
{
  double num = (x - mu);
  double denom = (sigma * sqrt(2.0));
  double frac = num / denom;
  double erfRes = boost::math::erf(frac);
  return (1 + erfRes) / 2.0;
}
