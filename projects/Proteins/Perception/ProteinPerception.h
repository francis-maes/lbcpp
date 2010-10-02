/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.h            | Protein Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_H_
# define LBCPP_PROTEIN_PERCEPTION_H_

# include <lbcpp/Function/Perception.h>
# include "../Data/Protein.h"

namespace lbcpp
{

/*
** ProteinFunction
*/
extern FunctionPtr proteinToVariableFunction(int);
extern FunctionPtr residueToSelectPairSequencesFunction(int index1, int index2);

/*
** ProteinPerception
*/
class ProteinCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual String getPreferedOutputClassName() const
    {return T("protein");}
};

inline PerceptionPtr proteinLengthPerception()
  {return functionBasedPerception(proteinLengthFunction());}

/*
** ResiduePerception
*/
class ResiduePerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), integerType());}
};

typedef ReferenceCountedObjectPtr<ResiduePerception> ResiduePerceptionPtr;
  
class PositionResiduePerception : public ResiduePerception
{
public:
  virtual size_t getNumOutputVariables() const
    {return 1;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return probabilityType();}
  
  virtual String getOutputVariableName(size_t index) const
    {return T("Position");}
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ProteinPtr protein = input[0].getObjectAndCast<Protein>();
    jassert(protein);
    callback->sense(0, Variable((double)input[1].getInteger() / protein->getLength(), probabilityType()));
  }
};
  
typedef ReferenceCountedObjectPtr<PositionResiduePerception> PositionResiduePerceptionPtr;

extern ResiduePerceptionPtr positionResiduePerception();

class IndexResiduePerception : public ResiduePerception
{
public:
  virtual size_t getNumOutputVariables() const
    {return 1;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return integerType();}
  
  virtual String getOutputVariableName(size_t index) const
    {return T("Index");}
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {callback->sense(0, input[1]);}
};

typedef ReferenceCountedObjectPtr<IndexResiduePerception> IndexResiduePerceptionPtr;

extern ResiduePerceptionPtr indexResiduePerception();

class TerminusProximityResiduePerception : public ResiduePerception
{
public:
  TerminusProximityResiduePerception(size_t outOfBoundWindowSize = 10)
    : outOfBoundWindowSize(outOfBoundWindowSize) {}
  
  virtual size_t getNumOutputVariables() const
    {return 1;}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return probabilityType();}
  
  virtual String getOutputVariableName(size_t index) const
    {return T("Terminus");}
  
  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
  {
    ProteinPtr protein = input[0].getObjectAndCast<Protein>();
    jassert(protein);
    size_t n = protein->getLength();
    size_t index = input[1].getInteger();
    
    if (index < outOfBoundWindowSize)
      callback->sense(0, Variable(index * (0.5 / outOfBoundWindowSize), probabilityType()));
    else if (n - index - 1 < outOfBoundWindowSize)
      callback->sense(0, Variable(0.5 + (outOfBoundWindowSize - (n - index - 1)) * (0.5 / outOfBoundWindowSize), probabilityType()));
    else
      callback->sense(0, Variable(0.5, probabilityType()));
  }

protected:
  friend class TerminusProximityResiduePerceptionClass;

  size_t outOfBoundWindowSize;
};

typedef ReferenceCountedObjectPtr<TerminusProximityResiduePerception> TerminusProximityResiduePerceptionPtr;

extern ResiduePerceptionPtr terminusProximityResiduePerception(size_t outOfBoundWindowSize);
  
class ResidueCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), integerType());}

  virtual String getPreferedOutputClassName() const
    {return T("residue");}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

extern DecoratorPerceptionPtr proteinToResiduePerception(PerceptionPtr proteinPerception);

/*
** ResiduePairPerception
*/
class ResiduePairPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}
};

typedef ReferenceCountedObjectPtr<ResiduePairPerception> ResiduePairPerceptionPtr;

class ResiduePairCompositePerception : public CompositePerception
{
public:
  virtual TypePtr getInputType() const
    {return pairType(proteinClass(), pairType(integerType(), integerType()));}
  virtual String getPreferedOutputClassName() const
    {return T("(ResiduePairPerception)");}

  virtual void addPerception(const String& name, PerceptionPtr subPerception);
};

extern DecoratorPerceptionPtr proteinToResiduePairPerception(PerceptionPtr proteinPerception);
extern ResiduePairPerceptionPtr residueToResiduePairPerception(PerceptionPtr residuePerception);

extern ResiduePairPerceptionPtr separationDistanceResiduePairPerception();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
