/*-----------------------------------------.---------------------------------.
| Filename: ProteinModel.h                 |                                 |
| Author  : Julien Becker                  |                                 |
| Started : 08/02/2012 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef _PROTEINS_MODEL_H_
# define _PROTEINS_MODEL_H_

# include <lbcpp/Core/CompositeFunction.h>
# include "../Data/Protein.h"

namespace lbcpp
{

class Model : public CompositeFunction
{
public:
  // Input -> Perception
  virtual FunctionPtr createPerception(ExecutionContext& context) const = 0;

  // Structured(Perception), Structured(Supervision) -> Structured(Output)
  virtual FunctionPtr createPredictor(ExecutionContext& context) const = 0;

protected:
  // Input, Supervision -> Output
  virtual void buildFunction(CompositeFunctionBuilder& builder);  
};

typedef ReferenceCountedObjectPtr<Model> ModelPtr;

class ProteinModel : public Model
{
public:
  ProteinModel(ProteinTarget target)
    : target(target) {}

  ProteinTarget getProteinTarget() const
    {return target;}

  // Protein -> DoubleVector[Perception]
  virtual FunctionPtr createPerception(ExecutionContext& context) const;

  // Structured(DoubleVector[Perception]), Structured(Supervision) -> Structured(Output)
  virtual FunctionPtr createPredictor(ExecutionContext& context) const;

  // Protein(Input), Protein(Supervision) -> Protein(Output)
  virtual void buildFunction(CompositeFunctionBuilder& builder);

protected:
  friend class ProteinModelClass;

  ProteinTarget target;

  // DoubleVector[Perception] -> Output
  virtual FunctionPtr createMachineLearning(ExecutionContext& context) const = 0;

  void buildPerception(CompositeFunctionBuilder& builder) const;
  // ProteinMap -> ConcatenateDoubleVector
  void buildGlobalPerception(CompositeFunctionBuilder& builder) const;
  // PositiveInteger(Position), ProteinMap -> ConcatenateDoubleVector
  void buildResiduePerception(CompositeFunctionBuilder& builder) const;

  // PositiveInteger(Cysteine Position), ProteinMap -> ConcatenateDoubleVector
  virtual void buildCysteineResiduePerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

  // PositiveInteger(Cysteine Position), PositiveInteger(Cysteine Position), ProteinMap -> ConcatenateDoubleVector
  virtual void buildCysteineResiduePairPerception(CompositeFunctionBuilder& builder) const
    {jassertfalse;}

  // ProteinMap -> DoubleVector
  virtual void globalFeatures(CompositeFunctionBuilder& builder) const = 0;
  // PositiveInteger(Position), ProteinMap -> DoubleVector
  virtual void residueFeatures(CompositeFunctionBuilder& builder) const = 0;
};

extern ClassPtr proteinModelClass;
typedef ReferenceCountedObjectPtr<ProteinModel> ProteinModelPtr;

class ProteinParallelModel : public CompositeFunction
{
public:
  void addModel(ProteinModelPtr model)
    {jassert(model); models.push_back(model);}

  // Protein(Input), Protein(Supervision) -> Protein(Output)
  virtual void buildFunction(CompositeFunctionBuilder& builder);

protected:
  friend class ProteinParallelModelClass;

  std::vector<ProteinModelPtr> models;
};

typedef ReferenceCountedObjectPtr<ProteinParallelModel> ProteinParallelModelPtr;

class ProteinSequentialModel : public CompositeFunction
{
public:
  void addModel(ProteinModelPtr model)
    {jassert(model); models.push_back(model);}

  void addParallelModel(ProteinParallelModelPtr model)
    {jassert(model); models.push_back(model);}

  // Protein(Input), Protein(Supervision) -> Protein(Output)
  virtual void buildFunction(CompositeFunctionBuilder& builder);

protected:
  friend class ProteinSequentialModelClass;

  std::vector<CompositeFunctionPtr> models;
};

typedef ReferenceCountedObjectPtr<ProteinSequentialModel> ProteinSequentialModelPtr;

}; /* namespace lbcpp */

#endif // _PROTEINS_MODEL_H_
