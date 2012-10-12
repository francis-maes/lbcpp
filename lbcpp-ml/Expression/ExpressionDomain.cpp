/*-----------------------------------------.---------------------------------.
| Filename: ExpressionDomain.cpp           | Expression Domain               |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp-ml/ExpressionDomain.h>
#include "../../lbcpp-core/Luape/NodeBuilder/NodeBuilderDecisionProblem.h"
using namespace lbcpp;

ExpressionDomain::ExpressionDomain(ExpressionUniversePtr universe)
  : universe(universe)
{
  if (!universe)
    this->universe = new ExpressionUniverse();
}

String ExpressionDomain::toShortString() const
{
  String res;
  if (inputs.size())
  {
    res += "Variables: ";
    for (size_t i = 0; i < inputs.size(); ++i)
    {
      res += inputs[i]->toShortString();
      if (i < inputs.size() - 1)
        res += ", ";
    }
    res += "\n";
  }
  else
    res += "No inputs\n";
  if (constants.size())
  {
    res += "Constants: ";
    for (size_t i = 0; i < constants.size(); ++i)
    {
      res += constants[i]->toShortString();
       if (i < constants.size() - 1)
        res += ", ";
    }
    res += "\n";
  }
  else
    res += "No constants\n";
  if (functions.size())
  {
    res += "Functions: ";
    for (size_t i = 0; i < functions.size(); ++i)
    {
      res += functions[i]->toShortString();
       if (i < functions.size() - 1)
        res += ", ";
    }
    res += "\n";
  }
  else
    res += "No functions\n";
  return res;
}

VariableExpressionPtr ExpressionDomain::addInput(const TypePtr& type, const String& name)
{
  size_t index = inputs.size();
  VariableExpressionPtr res(new VariableExpression(type, name, index));
  inputs.push_back(res);
  return res;
}

VariableExpressionPtr ExpressionDomain::createSupervision(const TypePtr& type, const String& name)
{
  supervision = new VariableExpression(type, name, inputs.size());
  return supervision;
}

ExpressionPtr ExpressionDomain::getActiveVariable(size_t index) const
{
  jassert(index < activeVariables.size());
  std::set<ExpressionPtr>::const_iterator it = activeVariables.begin();
  for (size_t i = 0; i < index; ++i)
    ++it;
  return *it;
}

bool ExpressionDomain::isTargetTypeAccepted(TypePtr type)
{
  if (!targetTypes.size())
  {
    // old definition for Luape weak learners
    return type->inheritsFrom(booleanType) || type->inheritsFrom(doubleType) ||
      (!type.isInstanceOf<Enumeration>() && type->inheritsFrom(integerType));
  }

  for (std::set<TypePtr>::const_iterator it = targetTypes.begin(); it != targetTypes.end(); ++it)
    if (type->inheritsFrom(*it))
      return true;
  return false;
}

size_t ExpressionDomain::getMaxFunctionArity() const
{
  size_t res = 0;
  for (size_t i = 0; i < functions.size(); ++i)
  {
    size_t arity = functions[i]->getNumInputs();
    if (arity > res)
      res = arity;
  }
  return res;
}

PostfixExpressionTypeSpacePtr ExpressionDomain::getSearchSpace(ExecutionContext& context, size_t complexity, bool verbose) const
{
  ScopedLock _(typeSearchSpacesLock);

  ExpressionDomain* pthis = const_cast<ExpressionDomain* >(this);

  if (complexity >= typeSearchSpaces.size())
    pthis->typeSearchSpaces.resize(complexity + 1);
  if (typeSearchSpaces[complexity])
    return typeSearchSpaces[complexity];

  return (pthis->typeSearchSpaces[complexity] = createTypeSearchSpace(context, std::vector<TypePtr>(), complexity, verbose));
}

PostfixExpressionTypeSpacePtr ExpressionDomain::createTypeSearchSpace(ExecutionContext& context, const std::vector<TypePtr>& initialState, size_t complexity, bool verbose) const
{
  PostfixExpressionTypeSpacePtr res = new PostfixExpressionTypeSpace(refCountedPointerFromThis(this), initialState, complexity);
  res->pruneStates(context, verbose);
  res->assignStateIndices(context);
  return res;
}

static void enumerateExhaustively(ExecutionContext& context, ExpressionBuilderStatePtr state, std::vector<ExpressionPtr>& res, bool verbose)
{
  if (state->isFinalState() && state->getStackSize() == 1)
  {
    ExpressionPtr node = state->getStackElement(0);
    res.push_back(node);
    //if (verbose)
    //  context.informationCallback(node->toShortString());
  }
  else
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      Variable stateBackup;
      Variable action = actions->getElement(i);
      double reward;
      state->performTransition(context, action, reward, &stateBackup);
      enumerateExhaustively(context, state, res, verbose);
      state->undoTransition(context, stateBackup);
    }
  }
}

void ExpressionDomain::enumerateNodesExhaustively(ExecutionContext& context, size_t complexity, std::vector<ExpressionPtr>& res, bool verbose, const PostfixExpressionSequencePtr& subSequence) const
{
  PostfixExpressionTypeSpacePtr typeSearchSpace;
  if (subSequence)
    typeSearchSpace = createTypeSearchSpace(context, subSequence->computeTypeState(), complexity, verbose); // create on-demand
  else
    typeSearchSpace = getSearchSpace(context, complexity, verbose); // use cached version

  ExpressionBuilderStatePtr state = new ExpressionBuilderState(refCountedPointerFromThis(this), typeSearchSpace, subSequence);
  enumerateExhaustively(context, state, res, verbose);
}

LuapeSamplesCachePtr ExpressionDomain::createCache(size_t size, size_t maxCacheSizeInMb) const
    {return new LuapeSamplesCache(universe, inputs, size, maxCacheSizeInMb);}

/*
** ExpressionProblem
*/
ExpressionProblem::ExpressionProblem()
{
  domain = new ExpressionDomain();
  std::vector< std::pair<double, double> > limits(1);
  limits[0] = std::make_pair(-DBL_MAX, DBL_MAX);
  //limits[1] = std::make_pair(DBL_MAX, 0); // expression size: should be minimized
  this->limits = new FitnessLimits(limits);
}

ObjectPtr ExpressionProblem::proposeStartingSolution(ExecutionContext& context) const
  {jassertfalse; return ExpressionPtr();}
    
bool ExpressionProblem::loadFromString(ExecutionContext& context, const String& str)
{
  if (!Problem::loadFromString(context, str))
    return false;
  initialize();
  return true;
}
