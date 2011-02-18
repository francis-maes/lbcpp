/*-----------------------------------------.---------------------------------.
| Filename: CompositeFunction.cpp          | Composite Function              |
| Author  : Francis Maes                   |                                 |
| Started : 18/02/2011 19:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Core/CompositeFunction.h>
using namespace lbcpp;

/*
** CompositeFunction
*/
TypePtr CompositeFunction::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  stateClass = new DynamicClass(getClassName() + T("State"));

  // build composite function
  CompositeFunctionBuilder builder(context, refCountedPointerFromThis(this), inputVariables);
  buildFunction(builder);
  if (builder.hasFailed())
    return TypePtr();

  // fill output variable signature
  VariableSignaturePtr lastMemberVariable = stateClass->getMemberVariable(stateClass->getNumMemberVariables() - 1);
  outputName = lastMemberVariable->getName();
  outputShortName = lastMemberVariable->getShortName();
  return lastMemberVariable->getType();
}

Variable CompositeFunction::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  std::vector<Variable> state(functions.size());
  //std::vector<Variable> subInputs(maxNumFunctionInputs);

  juce::int64* tmp = (juce::int64* )malloc(sizeof (Variable) * maxNumFunctionInputs);
  jassert(sizeof (Variable) == 2 * sizeof (juce::int64));

  for (size_t i = 0; i < functions.size(); ++i)
  {
    const FunctionPtr& function = functions[i];
    const std::vector<size_t>& inputIndices = functionInputs[i];
    juce::int64* ptr = tmp;

    for (size_t j = 0; j < inputIndices.size(); ++j)
    {
      size_t index = inputIndices[j];
      StepType stepType = steps[index].first;
      size_t stepArgument = steps[index].second;
      const Variable* var;
      if (stepType == inputStep)
        var = &inputs[stepArgument];
      else if (stepType == constantStep)
        var = &constants[stepArgument];
      else if (stepType == functionStep)
        var = &state[stepArgument];
      else
        jassert(false);

      const juce::int64* vari = (const juce::int64* )var;
      *ptr++ = vari[0];
      *ptr++ = vari[1];
      //subInputs[j] = *var;
    }
    state[i] = function->compute(context, (const Variable* )tmp);//&subInputs[0]);
  }
  free(tmp);
  jassert(steps.back().first == functionStep);
  return state.back();
}

ObjectPtr CompositeFunction::makeInitialState(const ObjectPtr& inputsObject) const
{
  DenseGenericObjectPtr res = new DenseGenericObject(stateClass);
  for (size_t i = 0; i < steps.size(); ++i)
  {
    StepType stepType = steps[i].first;
    size_t stepArgument = steps[i].second;
    if (stepType == inputStep)
      res->setVariable(i, inputsObject->getVariable(stepArgument));
    else if (stepType == constantStep)
      res->setVariable(i, constants[stepArgument]);
  }
  return res;
}

ObjectPtr CompositeFunction::makeSubInputsObject(size_t stepNumber, const ObjectPtr& state) const
{
  jassert(stepNumber < steps.size() && steps[stepNumber].first == functionStep);
  size_t functionIndex = steps[stepNumber].second;
  const FunctionPtr& function = functions[functionIndex];
  const std::vector<size_t>& inputIndices = functionInputs[functionIndex];

  ObjectPtr res = function->getInputsClass();
  for (size_t i = 0; i < inputIndices.size(); ++i)
    res->setVariable(i, state->getVariable(inputIndices[i]));
  return res;
}

void CompositeFunction::updateState(ExecutionContext& context, size_t stepNumber, ObjectPtr& state) const
{
  jassert(stepNumber < steps.size() && steps[stepNumber].first == functionStep);
  size_t functionIndex = steps[stepNumber].second;
  const FunctionPtr& function = functions[functionIndex];
  const std::vector<size_t>& inputIndices = functionInputs[functionIndex];
  
  if (inputIndices.size() == 1)
  {
    // optimized function for one input
    Variable v = state->getVariable(inputIndices[0]);
    state->setVariable(stepNumber, function->compute(context, &v));
  }
  else
  {
    std::vector<Variable> inputs(inputIndices.size());
    for (size_t i = 0; i < inputs.size(); ++i)
      inputs[i] = state->getVariable(inputIndices[i]);
    state->setVariable(stepNumber, function->compute(context, inputs));
  }
}

/*
** CompositeFunctionBuilder
*/
CompositeFunctionBuilder::CompositeFunctionBuilder(ExecutionContext& context, CompositeFunctionPtr function, const std::vector<VariableSignaturePtr>& inputVariables)
  : context(context), function(function), inputVariables(inputVariables), numInputs(0), failed(false)
{
}

size_t CompositeFunctionBuilder::addVariable(TypePtr type, const String& name, const String& shortName, CompositeFunction::StepType stepType, size_t stepArgument)
{
  size_t res = function->steps.size();
  function->stateClass->addMemberVariable(context, type, name, shortName);
  function->steps.push_back(std::make_pair(stepType, stepArgument));
  currentSelection.push_back(res);
  return res;
}

size_t CompositeFunctionBuilder::addInput(TypePtr type, const String& optionalName, const String& optionalShortName)
{
  size_t stepArgument = numInputs++;
  const VariableSignaturePtr& inputVariable = inputVariables[stepArgument];
  if (!context.checkInheritance(inputVariable->getType(), type))
    return returnError();

  String name = optionalName.isNotEmpty() ? optionalName : inputVariable->getName();
  String shortName = optionalShortName.isNotEmpty() ? optionalShortName : inputVariable->getShortName();
  return addVariable(inputVariable->getType(), name, shortName, CompositeFunction::inputStep, stepArgument);
}

size_t CompositeFunctionBuilder::addConstant(const Variable& value, const String& name, const String& shortName)
{
  if (!value.exists())
  {
    context.errorCallback(T("Constant " ) + name.quoted() + T(" does not exists"));
    return returnError();
  }
  size_t stepArgument = function->constants.size();
  function->constants.push_back(value);
  return addVariable(value.getType(), name, shortName, CompositeFunction::constantStep, stepArgument);
}

size_t CompositeFunctionBuilder::addFunction(const FunctionPtr& function, size_t input, const String& outputName, const String& outputShortName)
  {return addFunction(function, std::vector<size_t>(1, input), outputName, outputShortName);}

size_t CompositeFunctionBuilder::addFunction(const FunctionPtr& function, size_t input1, size_t input2, const String& outputName, const String& outputShortName)
{
  std::vector<size_t> inputs(2);
  inputs[0] = input1;
  inputs[1] = input2;
  return addFunction(function, inputs, outputName, outputShortName);
}

size_t CompositeFunctionBuilder::addFunction(const FunctionPtr& subFunction, const std::vector<size_t>& inputs, const String& optionalName, const String& optionalShortName)
{
  std::vector<VariableSignaturePtr> inputVariables(inputs.size());
  for (size_t i = 0; i < inputVariables.size(); ++i)
  {
    size_t inputIndex = inputs[i];
    if (inputIndex >= function->steps.size())
    {
      context.errorCallback(T("Invalid input index: ") + String((int)inputIndex));
      return returnError();
    }
    inputVariables[i] = function->stateClass->getMemberVariable(inputIndex);
    if (!inputVariables[i])
    {
      context.errorCallback(T("Missing variable, index = ") + String((int)inputIndex));
      return returnError();
    }
  }
  if (!subFunction->initialize(context, inputVariables))
    return returnError();

  size_t stepArgument = function->functions.size();
  function->functions.push_back(subFunction);
  function->functionInputs.push_back(inputs);
  if (inputs.size() > function->maxNumFunctionInputs)
    function->maxNumFunctionInputs = inputs.size();

  VariableSignaturePtr signature = subFunction->getOutputVariable();
  String name = optionalName.isNotEmpty() ? optionalName : signature->getName();
  String shortName = optionalShortName.isNotEmpty() ? optionalShortName : signature->getShortName();
  return addVariable(signature->getType(), name, shortName, CompositeFunction::functionStep, stepArgument);
}

void CompositeFunctionBuilder::startSelection()
  {currentSelection.clear();}

const std::vector<size_t>& CompositeFunctionBuilder::finishSelection()
  {return currentSelection;}

size_t CompositeFunctionBuilder::finishSelectionWithFunction(const FunctionPtr& function)
{
  size_t res = addFunction(function, currentSelection);
  currentSelection.clear();
  return res;
}

TypePtr CompositeFunctionBuilder::getOutputType() const
{
  size_t n = function->stateClass->getNumMemberVariables();
  jassert(n);
  return function->stateClass->getMemberVariableType(n - 1);
}
