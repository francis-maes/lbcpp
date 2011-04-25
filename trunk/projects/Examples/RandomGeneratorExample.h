/*-----------------------------------------.---------------------------------.
| Filename: RandomGeneratorExample.h       | Random Generator Example        |
| Author  : Francis Maes                   |                                 |
| Started : 25/04/2011 11:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_RANDOM_GENERATOR_H_
# define LBCPP_EXAMPLES_RANDOM_GENERATOR_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp
{

class RandomGeneratorExample : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    static const size_t N = 1000000;

    RandomGeneratorPtr random = RandomGenerator::getInstance();
    ScalarVariableStatisticsPtr stats;

    context.enterScope(T("sampleBoolean(0.86)"));
    stats = new ScalarVariableStatistics(T("sampleBoolean"));
    for (size_t i = 0; i < N; ++i)
      stats->push(random->sampleBool(0.86) ? 1.0 : 0.0);
    context.leaveScope(stats);

    context.enterScope(T("sampleByte()"));
    stats = new ScalarVariableStatistics(T("sampleByte"));
    for (size_t i = 0; i < N; ++i)
      stats->push((double)random->sampleByte());
    context.leaveScope(stats);
    
    context.enterScope(T("sampleInt(100000)"));
    stats = new ScalarVariableStatistics(T("sampleInt"));
    for (size_t i = 0; i < N; ++i)
      stats->push((double)random->sampleInt(100000));
    context.leaveScope(stats);

    context.enterScope(T("sampleDouble()"));
    stats = new ScalarVariableStatistics(T("sampleDouble"));
    for (size_t i = 0; i < N; ++i)
      stats->push(random->sampleDouble());
    context.leaveScope(stats);

    context.enterScope(T("sampleDoubleFromGaussian(1664, 51)"));
    stats = new ScalarVariableStatistics(T("sampleDoubleFromGaussian"));
    for (size_t i = 0; i < N; ++i)
      stats->push(random->sampleDoubleFromGaussian(1664, 51));
    context.leaveScope(stats);

    context.enterScope(T("sampleDouble() all samples"));
    for (size_t i = 0; i < 1000; ++i)
    {
      context.enterScope(T("sample"));
      context.resultCallback(T("i"), i);
      context.resultCallback(T("sample"), random->sampleDoubleFromGaussian());
      context.leaveScope(true);
    }
    context.leaveScope(true);

    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_WORK_UNIT_H_
