/*-----------------------------------------.---------------------------------.
| Filename: Domain.h                       | Mathematical domain             |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_DOMAIN_H_
# define LBCPP_ML_DOMAIN_H_

# include "predeclarations.h"
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class Domain : public Object
{
public:
  virtual ObjectPtr projectIntoDomain(const ObjectPtr& object) const
    {return object;}
};

class ContinuousDomain : public Domain
{
public:
  ContinuousDomain(const std::vector< std::pair<double, double> >& limits)
    : limits(limits) {}
  ContinuousDomain() {}

  size_t getNumDimensions() const
    {return limits.size();}

  double getLowerLimit(size_t dimension) const
    {jassert(dimension < limits.size()); return limits[dimension].first;}
  
  double getUpperLimit(size_t dimension) const
    {jassert(dimension < limits.size()); return limits[dimension].second;}

  const std::vector< std::pair<double, double> >& getLimits() const
    {return limits;}

  void setLimits(size_t dimension, double lowest, double highest)
    {jassert(dimension < limits.size()); limits[dimension] = std::make_pair(lowest, highest);}

  DenseDoubleVectorPtr sampleUniformly(RandomGeneratorPtr random) const;
  virtual ObjectPtr projectIntoDomain(const ObjectPtr& object) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class ContinuousDomainClass;

  std::vector< std::pair<double, double> > limits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_DOMAIN_H_
