/*-----------------------------------------.---------------------------------.
| Filename: ProteinFrame.h                 | Protein Frame                   |
| Author  : Francis Maes                   |                                 |
| Started : 28/01/2011 14:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_FRAME_H_
# define LBCPP_PROTEIN_FRAME_H_

# include <lbcpp/lbcpp.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class FunctionBuilder
{
public:
  FunctionBuilder(ExecutionContext& context, CompositeFunctionPtr function, const std::vector<VariableSignaturePtr>& inputVariables);

  size_t getNumProvidedInputs() const
    {return inputVariables.size();}

  VariableSignaturePtr getProvidedInput(size_t index) const
    {jassert(index < inputVariables.size()); return inputVariables[index];}

  size_t invalidIndex() const
    {return (size_t)-1;}

  size_t addInput(TypePtr type, const String& name = String::empty);

  size_t addConstant(const Variable& value, const String& name = String::empty);

  size_t addFunction(const FunctionPtr& function, size_t input, const String& outputName = String::empty, const String& outputShortName = String::empty);
  size_t addFunction(const FunctionPtr& function, size_t input1, size_t input2, const String& outputName = String::empty, const String& outputShortName = String::empty);
  size_t addFunction(const FunctionPtr& function, std::vector<size_t>& inputs, const String& outputName = String::empty, const String& outputShortName = String::empty);

  void startSelection();
  const std::vector<size_t>& finishSelection();
  size_t finishSelectionWithFunction(const FunctionPtr& function);

private:
  ExecutionContext& context;
  FrameClassPtr frameClass;
  std::vector<size_t> currentSelection;
  std::vector<VariableSignaturePtr> inputVariables;

  size_t addInSelection(size_t index);
};

class CompositeFunction : public FrameBasedFunction
{
public:
  virtual void buildFunction(FunctionBuilder& builder) = 0;

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    FunctionBuilder builder(context, refCountedPointerFromThis(this), inputVariables);
    buildFunction(builder);
    return FrameBasedFunction::initializeFunction(context, inputVariables, outputName, outputShortName);
  }
};

typedef ReferenceCountedObjectPtr<CompositeFunction> CompositeFunctionPtr;

typedef void (Object::*FunctionBuildFunction)(FunctionBuilder& builder) const; 

class MethodBasedCompositeFunction : public CompositeFunction
{
public:
  MethodBasedCompositeFunction(ObjectPtr factory, FunctionBuildFunction buildFunctionFunction)
    : factory(factory), buildFunctionFunction(buildFunctionFunction) {}

  virtual void buildFunction(FunctionBuilder& builder)
  {
    Object& object = *factory;
    (object.*buildFunctionFunction)(builder);
    factory = ObjectPtr(); // free reference to factory
  }

protected:
  ObjectPtr factory;
  FunctionBuildFunction buildFunctionFunction;
};

/////////////////////////////////////////////////
/////////////////////////////////////////////////

class ProteinFunctionFactory : public Object
{
public:
  virtual void residuePerceptions(FunctionBuilder& builder) const = 0;
};

class NumericalProteinFunctionFactory : public ProteinFunctionFactory
{
public:
  virtual void primaryResidueFeatures(FunctionBuilder& builder) const;
  virtual void primaryResidueFeaturesVector(FunctionBuilder& builder) const;
  virtual void residueFeatures(FunctionBuilder& builder) const;
  virtual void residueFeaturesVector(FunctionBuilder& builder) const;

  virtual void residuePerceptions(FunctionBuilder& builder) const
    {residueFeaturesVector(builder);}

  typedef void (NumericalProteinFunctionFactory::*ThisClassFunctionBuildFunction)(FunctionBuilder& builder) const; 

  FunctionPtr function(ThisClassFunctionBuildFunction buildFunc) const
    {return function((FunctionBuildFunction)buildFunc);}

  FunctionPtr function(FunctionBuildFunction buildFunc) const
    {return new MethodBasedCompositeFunction(refCountedPointerFromThis(this), buildFunc);}
};

typedef ReferenceCountedObjectPtr<NumericalProteinFunctionFactory> NumericalProteinFunctionFactoryPtr;

extern FunctionPtr proteinResidueFeaturesVectorFunction();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
