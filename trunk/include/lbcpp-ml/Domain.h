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
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

class Domain : public Object
{
public:
  virtual SamplerPtr createDefaultSampler() const
    {return SamplerPtr();}

  virtual ObjectPtr projectIntoDomain(const ObjectPtr& object) const
    {return object;}
};

class DiscreteDomain : public Domain
{
public:
  size_t getNumElements() const
    {return elements.size();}

  const ObjectPtr& getElement(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

  void addElement(const ObjectPtr& object);
  void addElements(const DiscreteDomainPtr& domain);

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

private:
  friend class DiscreteDomainClass;

  std::vector<ObjectPtr> elements;
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

class VectorDomain : public Domain
{
public:
  VectorDomain(DomainPtr elementsDomain = DomainPtr())
    : elementsDomain(elementsDomain) {}

  DomainPtr getElementsDomain() const
    {return elementsDomain;}

protected:
  friend class VectorDomainClass;

  DomainPtr elementsDomain;
};

extern DomainPtr vectorDomain(DomainPtr elementsDomain);

}; /* namespace lbcpp */

#endif // !LBCPP_ML_DOMAIN_H_
