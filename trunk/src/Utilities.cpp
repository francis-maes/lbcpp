/*-----------------------------------------.---------------------------------.
| Filename: Utilities.cpp                  | Utilities                       |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lcpp/Utilities.h>
#include <lcpp/Random.h>
#include <lcpp/IterationFunction.h>
#include <iostream>
#include <fstream>
using namespace lcpp;

/*
** ProgressCallback
*/
class ConsoleProgressCallback : public ProgressCallback
{
public:
  virtual void progressBegin(const std::string& description)
    {std::cout << "Begin '" << description << "'" << std::endl;}
    
  // return false to stop the task
  virtual bool progressStep(const std::string& description, double iteration, double totalIterations = 0)
  {
    std::cout << "Step '" << description << "' iteration = " << iteration;
    if (totalIterations)
      std::cout << " / " << totalIterations;
    std::cout << std::endl;
    return true;
  }
    
  virtual void progressEnd()
    {std::cout << "End." << std::endl;}
};

static ConsoleProgressCallback consoleProgressCallback;

ProgressCallback& ProgressCallback::getConsoleProgressCallback()
  {return consoleProgressCallback;}

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
  assert(false);
  return 0;
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

/*
** IterationFunction
*/
class ConstantIterationFunction : public IterationFunction
{
public:
  ConstantIterationFunction(double value) : value(value) {}
  
  virtual double compute(size_t iteration) const
    {return value;}
    
private:
  double value;
};

IterationFunctionPtr IterationFunction::createConstant(double value)
  {return new ConstantIterationFunction(value);}

class InvLinearIterationFunction : public IterationFunction
{
public:
  InvLinearIterationFunction(double initialValue, size_t numberIterationsToReachHalfInitialValue)
    : initialValue(initialValue), numberIterationsToReachHalfInitialValue(numberIterationsToReachHalfInitialValue) {}
    
  virtual double compute(size_t iteration) const
    {return initialValue * numberIterationsToReachHalfInitialValue / (double)(numberIterationsToReachHalfInitialValue + iteration);}

private:
  double initialValue;
  size_t numberIterationsToReachHalfInitialValue;
};

IterationFunctionPtr IterationFunction::createInvLinear(double initialValue, size_t numberIterationsToReachHalfInitialValue)
  {return new InvLinearIterationFunction(initialValue, numberIterationsToReachHalfInitialValue);}

/*
** TextFileParser
*/
bool TextFileParser::parseLine(const std::string& line)
{
  size_t begin = line.find_first_not_of(" \t");
  bool isEmpty = begin == std::string::npos;
  if (isEmpty)
    return parseEmptyLine();
  if (line[begin] == '#')
    return parseCommentLine(line.substr(begin + 1));
  std::vector<std::string> columns;
  tokenize(line, columns);
  return parseDataLine(columns);
}

void TextFileParser::tokenize(const std::string& line, std::vector< std::string >& columns, const char* separators)
{
  //std::cout << "Tokenize " << lcpp::toString(line) << " => ";
  size_t b = line.find_first_not_of(separators);
  while (b != std::string::npos)
  {
    size_t e = line.find_first_of(separators, b);
    if (e == std::string::npos)
      columns.push_back(line.substr(b));
    else
      columns.push_back(line.substr(b, e - b));
    b = line.find_first_not_of(separators, e);
  }
  //std::cout << lcpp::toString(columns) << std::endl;
}

bool TextFileParser::parseStream(std::istream& istr)
{
  parsingBreaked = false;
  parseBegin();
  while (!istr.eof() && !parsingBreaked)
  {
    std::string line;
    std::getline(istr, line);
    size_t n = line.find_last_not_of("\r\n");
    if (n != std::string::npos)
      line = line.substr(0, n + 1);
    if (!parseLine(line))
    {
      Object::error("TextFileParser::parse", "Could not parse line '" + line + "'");
      return false;
    }
  }
  if (!parseEnd())
  {
    Object::error("TextFileParser::parse", "Error in parse end");
      return false;
  }
  return true;
}

bool TextFileParser::parseFile(const std::string& filename)
{
  if (filename == "")
  {
    Object::error("TextFileParser::parseFile", "No filename specified");
    return false;
  }
  std::ifstream istr(filename.c_str());
  if (!istr.is_open())
  {
    Object::error("TextFileParser::parseFile", "Could not open file '" + filename + "'");
    return false;
  }
  return parseStream(istr);
}
