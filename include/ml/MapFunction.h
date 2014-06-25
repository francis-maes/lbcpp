/*-----------------------------------------.---------------------------------.
| Filename: MapFunction.h                  | A function that maps an input   |
| Author  : Denny Verbeeck                 | list to an output list of equal |
| Started : 25/06/2014 11:25               | size                            |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_MAP_FUNCTION_H_
# define ML_MAP_FUNCTION_H_

# include "predeclarations.h"
# include <ml/RandomVariable.h>

namespace lbcpp
{

/*!
** @class MapFunction
** @brief A function that maps an input list to an output list of equal size
*/
class MapFunction : public Object
{
public:
  /*
  ** Map an input list to an output list
  */
  virtual std::vector<ObjectPtr> map(ExecutionContext& context, const std::vector<ObjectPtr>& list) const = 0;
  /*
  ** This method can be used to update function parameters
  */
  virtual void update(ExecutionContext& context, const std::vector<ObjectPtr>& objects) {}
};


class NormalizeMapFunction : public MapFunction
{
public:
  virtual std::vector<ObjectPtr> map(ExecutionContext& context, const std::vector<ObjectPtr>& list) const
  {
    std::vector<ObjectPtr> result;
    jassert(statistics.size() <= list.size());

    for (size_t i = 0; i < statistics.size(); ++i)
      result.push_back(new Double(normalize(Double::get(list[i]), statistics[i]->getMean(), statistics[i]->getStandardDeviation())));

    return result;
  }

  virtual void update(ExecutionContext& context, const std::vector<ObjectPtr>& list)
  {
    if (statistics.empty())
      for (size_t i = 0; i < list.size(); ++i)
        statistics.push_back(new ScalarVariableMeanAndVariance());
    jassert(statistics.size() == list.size());
    for (size_t i = 0; i < list.size(); ++i)
      statistics[i]->push(Double::get(list[i]));
  }

protected:
  std::vector<ScalarVariableMeanAndVariancePtr> statistics;  /**< Parameters for normalization */

  inline double normalize(double input, double mean, double stddev) const
    {return (input - mean) / (3*stddev);}
};

typedef ReferenceCountedObjectPtr<MapFunction> MapFunctionPtr;

} /* namespace lbcpp */

#endif //!ML_MAP_FUNCTION_H_
