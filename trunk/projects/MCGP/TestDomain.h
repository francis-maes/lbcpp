/*-----------------------------------------.---------------------------------.
| Filename: TestDomain.h                   | Tests on Domains modeling       |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2012 11:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MCGP_TEST_DOMAIN_H_
# define LBCPP_MCGP_TEST_DOMAIN_H_

# include <lbcpp-ml/Domain.h>
# include <lbcpp-ml/Sampler.h>
# include <climits>

#if 0

namespace ocaml
{

using lbcpp::Object;
using lbcpp::ReferenceCountedObjectPtr;

class TypeParam;
typedef ReferenceCountedObjectPtr<TypeParam> TypeParamPtr;
class TypeParams;
typedef ReferenceCountedObjectPtr<TypeParams> TypeParamsPtr;
class TypeDefinition;
typedef ReferenceCountedObjectPtr<TypeDefinition> TypeDefinitionPtr;
class TypeDefinitions;
typedef ReferenceCountedObjectPtr<TypeDefinitions> TypeDefinitionsPtr;

class TypeExpression;
typedef ReferenceCountedObjectPtr<TypeExpression> TypeExpressionPtr;

class TypeParam : public Object
{
private:
  String identifer;
};

class TypeParams : public Object
{
public:

private:
  std::vector<TypeParamPtr> param;
};

// typedef ::=	[type-params] typeconstr-name [type-information]  
class TypeDefinition : public Object
{
private:
  std::vector<TypeParams> params;
  String typeConstructorName;
  // FIXME: type-information
};

// type-definition ::=	type typedef  { and typedef }  
class TypeDefinitions : public Object
{
public:

private:
  std::vector<TypeDefinitionPtr> typeDefs;
};

//////////////////////////

class TypeExpression : public Object
{
public:
};

// typexpr ::= 'ident | _
class IdentTypeExpression : public TypeExpression
{
public:

private:
  String ident;
};

// typeexpr ::= typeexpr -> typeexpr
class FunctionTypeExpression : public TypeExpression
{
private:
  TypeExpressionPtr left;
  TypeExpressionPtr right;
};

// typeexpr ::= typexpr  { * typexpr }+  
class TupleTypeExpression : public TypeExpression
{
private:
  std::vector<TypeExpressionPtr> types;
};

// typeexpr ::= typexpr typeconstr  |  ( typexpr  { , typexpr } ) typeconstr  
class ConstructedTypeExpression : public TypeExpression
{
private:
  std::vector<TypeExpressionPtr> arguments;
  String constructor;
};

}; /* namespace ocaml */

#endif // 0
#if 0

Example1:



let x = [Flag10; Flag3; Flag6]

Example2:

type simupol = RandomSimulationPolicy
type mcalgo = Simulate of simupol | Repeat of mcalgo * int | Step of mcalgo ...

#endif // 0


namespace lbcpp
{

/*
** Nil
*/
class NilObject : public Object
{
public:
};

typedef ReferenceCountedObjectPtr<NilObject> NilObjectPtr;

NilObjectPtr nilObject;

class NilDomain : public Domain
{
public:
  virtual SamplerPtr createDefaultSampler() const;
};

typedef ReferenceCountedObjectPtr<NilDomain> NilDomainPtr;

NilDomainPtr nilDomain;

class NilSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
    {}

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return nilObject;}

  virtual bool isDeterministic() const
    {return true;}
};

inline SamplerPtr NilDomain::createDefaultSampler() const
  {return new NilSampler();}

/*
** Integer
*/
class IntegerDomain : public Domain
{
public:
  IntegerDomain(int lowestValue = INT_MIN, int highestValue = INT_MAX)
    : lowestValue(lowestValue), highestValue(highestValue) {}

protected:
  friend class IntegerDomainClass;

  int lowestValue;
  int highestValue;
};

/*
** Double
*/
class DoubleObject : public Object
{
public:
  DoubleObject() : value(0.0) {}

private:
  double value;
};

class DoubleDomain : public Domain
{
public:
  DoubleDomain(double lowestValue = -DBL_MAX, double highestValue = DBL_MAX)
    : lowestValue(lowestValue), highestValue(highestValue) {}

  bool isFinite() const
    {return lowestValue != -DBL_MAX && highestValue != DBL_MAX;}

  virtual SamplerPtr createDefaultSampler() const;

private:
  friend class DoubleDomainClass;

  double lowestValue;
  double highestValue;
};

typedef ReferenceCountedObjectPtr<DoubleDomain> DoubleDomainPtr;

class UniformDoubleSampler : public Sampler
{
public:
  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<DoubleDomain>();
    jassert(this->domain->isFinite());
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
    {return nilObject;}

  virtual bool isDeterministic() const
    {return true;}

protected:
  DoubleDomainPtr domain;
};

inline SamplerPtr DoubleDomain::createDefaultSampler() const
{
  return isFinite() ? new UniformDoubleSampler() : SamplerPtr();
}

/*
** Tuple
*/
class TupleObject : public Object
{
public:
  TupleObject(const std::vector<ObjectPtr>& elements)
    : elements(elements) {}

private:
  std::vector<ObjectPtr> elements;
};

class TupleDomain : public Domain
{
public:
  TupleDomain(const DomainPtr& domain1, const DomainPtr& domain2, const DomainPtr& domain3)
    : elements(3)
  {
    elements[0] = domain1;
    elements[1] = domain2;
    elements[2] = domain3;
  }

  TupleDomain(const DomainPtr& domain1, const DomainPtr& domain2)
    : elements(2)
  {
    elements[0] = domain1;
    elements[1] = domain2;
  }

  void addElement(const DomainPtr& domain)
    {elements.push_back(domain);}

  size_t getNumElements() const
    {return elements.size();}

  DomainPtr getElement(size_t index) const
    {jassert(index < elements.size()); return elements[index];}

protected:
  std::vector<DomainPtr> elements;
};

typedef ReferenceCountedObjectPtr<TupleDomain> TupleDomainPtr;

class IndependentTupleSampler : public Sampler
{
public:
  IndependentTupleSampler(const std::vector<SamplerPtr>& samplers)
    : samplers(samplers) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<TupleDomain>();
    for (size_t i = 0; i < samplers.size(); ++i)
      samplers[i]->initialize(context, this->domain->getElement(i));
  }
  
  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    std::vector<ObjectPtr> elements(domain->getNumElements());
    for (size_t i = 0; i < elements.size(); ++i)
      elements[i] = samplers[i]->sample(context);
    return new TupleObject(elements);
  }

protected:
  std::vector<SamplerPtr> samplers;
  TupleDomainPtr domain;
};

/*
** Variant
*/
class VariantDomain : public Domain
{
public:
  void addConstructor(const String& identifier, const DomainPtr& domain = nilDomain)
    {constructors.push_back(std::make_pair(identifier, domain));}

  size_t getNumConstructors() const
    {return constructors.size();}

  const String& getConstructorName(size_t index) const
    {jassert(index < constructors.size()); return constructors[index].first;}

protected:
  std::vector< std::pair<String, DomainPtr> > constructors;
};

typedef ReferenceCountedObjectPtr<VariantDomain> VariantDomainPtr;

class VariantObject : public Object
{
public:
  VariantObject(const String& constructor, const ObjectPtr& argument)
    : constructor(constructor), argument(argument) {}

private:
  String constructor;
  ObjectPtr argument;
};

class UniformVariantSampler : public Sampler
{
public:
  UniformVariantSampler(const std::vector<SamplerPtr>& subSamplers)
    : subSamplers(subSamplers) {}

  virtual void initialize(ExecutionContext& context, const DomainPtr& domain)
  {
    this->domain = domain.staticCast<VariantDomain>();
  }

  virtual ObjectPtr sample(ExecutionContext& context) const
  {
    size_t constructorIndex = context.getRandomGenerator()->sampleSize(domain->getNumConstructors());
    return new VariantObject(domain->getConstructorName(constructorIndex), ObjectPtr());
  }

  virtual void learn(ExecutionContext& context, const std::vector<ObjectPtr>& objects)
    {jassertfalse;}

  virtual void reinforce(ExecutionContext& context, const ObjectPtr& object)
    {jassertfalse;}

protected:
  std::vector<SamplerPtr> subSamplers;
  VariantDomainPtr domain;
};

/*
** -----------------------------------------------
*/
class Toto
{
public:
  DomainPtr makeColoDomain() const
  {
    // type flag = Flag1 | Flag2 | ... | Flag50
    // type flag_list = Nil | Cons of flag * flag_list

    VariantDomainPtr flagDomain = new VariantDomain();
    for (size_t i = 0; i < 50; ++i)
      flagDomain->addConstructor(String((int)i));
    VariantDomainPtr res = new VariantDomain();
    res->addConstructor("Nil");
    res->addConstructor("Cons", new TupleDomain(flagDomain, res));
    return res;
  }

  DomainPtr makeSymbolicRegressionDomain() const
  {
    // type expr = X | Constant of double | Log of expr | Exp of expr ... | Div of expr * expr

    VariantDomainPtr res = new VariantDomain();
    res->addConstructor("X");
    res->addConstructor("Constant", new DoubleDomain());
    res->addConstructor("Log", res);
    res->addConstructor("Exp", res);
    res->addConstructor("Sin", res);
    res->addConstructor("Cos", res);
    res->addConstructor("Add", new TupleDomain(res, res));
    res->addConstructor("Sub", new TupleDomain(res, res));
    res->addConstructor("Mul", new TupleDomain(res, res));
    res->addConstructor("Div", new TupleDomain(res, res));
    return res;
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_MCGP_TEST_DOMAIN_H_
