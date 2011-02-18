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

class ProteinFunctionFactory : public Object
{
public:
  virtual void residuePerceptions(CompositeFunctionBuilder& builder) const = 0;
};

class NumericalProteinFunctionFactory : public ProteinFunctionFactory
{
public:
  virtual void primaryResidueFeatures(CompositeFunctionBuilder& builder) const;
  virtual void primaryResidueFeaturesVector(CompositeFunctionBuilder& builder) const;
  virtual void residueFeatures(CompositeFunctionBuilder& builder) const;
  virtual void residueFeaturesVector(CompositeFunctionBuilder& builder) const;

  virtual void residuePerceptions(CompositeFunctionBuilder& builder) const
    {residueFeaturesVector(builder);}

  typedef void (NumericalProteinFunctionFactory::*ThisClassFunctionBuildFunction)(CompositeFunctionBuilder& builder) const; 

  FunctionPtr function(ThisClassFunctionBuildFunction buildFunc) const
    {return function((FunctionBuildFunction)buildFunc);}

  FunctionPtr function(FunctionBuildFunction buildFunc) const
    {return new MethodBasedCompositeFunction(refCountedPointerFromThis(this), buildFunc);}
};

typedef ReferenceCountedObjectPtr<NumericalProteinFunctionFactory> NumericalProteinFunctionFactoryPtr;

extern FunctionPtr proteinResidueFeaturesVectorFunction();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_FRAME_H_
