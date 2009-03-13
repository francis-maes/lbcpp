/*-----------------------------------------.---------------------------------.
| Filename: ErrorHandler.cpp               | Error handling                  |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/Object.h>
#include <cralgo/Random.h>
#include <iostream>
using namespace cralgo;

/*
** ErrorHandler
*/
class DefaultErrorHandler : public ErrorHandler
{
public:
  virtual void errorMessage(const std::string& where, const std::string& what)
    {std::cerr << "Error in '" << where << "': " << what << "." << std::endl;}  
  virtual void warningMessage(const std::string& where, const std::string& what)
    {std::cerr << "Warning in '" << where << "': " << what << "." << std::endl;}  
};

static DefaultErrorHandler defaultErrorHandler;

ErrorHandler* ErrorHandler::instance = &defaultErrorHandler;

/*
** Random
*/
void Random::setSeed(int seed1, int seed2)
{
  this->seed = ((long long)(seed1) * 2654435769UL + (long long)(seed2) * 3373259426UL) >> 5;
  sampleInt();
}

size_t Random::sampleWithProbabilities(const std::vector<double>& probabilities, double probabilitiesSum)
{
  assert(probabilities.size());
  if (!probabilitiesSum)
    for (size_t i = 0; i < probabilities.size(); ++i)
      probabilitiesSum += probabilities[i];
  double number = sampleDouble(probabilitiesSum);
  for (size_t i = 0; i < probabilities.size(); ++i)
  {
    double prob = probabilities[i];
    if (number <= prob)
      return i;
    number -= prob;
  }
  return probabilities.size() - 1;
}

int Random::sampleInt()
{
  seed = (seed * 0x5deece66dLL + 11) & 0xffffffffffffLL;
  return (int)(seed >> 16);
}

double Random::sampleDoubleFromGaussian()
{
  // from http://www.taygeta.com/random/gaussian.html
  double x1, x2, w;
  do
  {
    x1 = sampleDouble(-1.0, 1.0);
    x2 = sampleDouble(-1.0, 1.0);
    w = x1 * x1 + x2 * x2;
  } while (w >= 1.0);

  w = sqrt((-2.0 * log(w)) / w);
  return x1 * w; // (x2 * w) is another sample from the same gaussian
}

