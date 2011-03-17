/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.h            | Protein Perception              |
| Author  : Julien Becker                  |                                 |
| Started : 17/02/2011 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_H_
# define LBCPP_PROTEIN_PERCEPTION_H_

namespace lbcpp
{

enum ProteinPerceptionType
{
  noProteinPerceptionType,
  residueType,
  residuePairType,
  disulfideBondType
};

class ProteinPerception : public Object
{
public:
  ProteinPerception(ProteinPtr protein)
    : protein(protein) {}
  ProteinPerception() {}
  
  void setLenght(size_t length)
    {this->length = length;}

  void setPrimaryResidueFeatures(VectorPtr features)
    {this->primaryResidueFeatures = features;}

  void setAccumulator(ContainerPtr accumulator)
    {this->accumulator = accumulator;}

  void setGlobalFeatures(DoubleVectorPtr globalFeatures)
    {this->globalFeatures = globalFeatures;}

  static ProteinPerceptionType typeOfProteinPerception(ProteinTarget target)
  {
    switch (target) {
      case ss3Target:
      case ss8Target:
      case stalTarget:
      case saTarget:
      case sa20Target:
      case drTarget: return residueType;
      case cma8Target:
      case cmb8Target:
      case dmaTarget:
      case dmbTarget: return residuePairType;
      case dsbTarget: return disulfideBondType;
      default:
        jassertfalse;
        return noProteinPerceptionType;
    }
  }
  
protected:
  friend class ProteinPerceptionClass;

  ProteinPtr protein;
  size_t length;
  VectorPtr primaryResidueFeatures;
  ContainerPtr accumulator;
  DoubleVectorPtr globalFeatures;
};

typedef ReferenceCountedObjectPtr<ProteinPerception> ProteinPerceptionPtr;

extern ClassPtr proteinPerceptionClass;

class CreateProteinPerceptionFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 5;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
    switch (index)
    {
      case 0: return proteinClass;
      case 1: return positiveIntegerType;
      case 2: return vectorClass(doubleVectorClass());
      case 3: return containerClass(doubleVectorClass());
      case 4: return doubleVectorClass();
      default:
        jassertfalse;
        return TypePtr();
    }
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return proteinPerceptionClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ProteinPerceptionPtr res = new ProteinPerception(inputs[0].getObject());
    res->setLenght((size_t)inputs[1].getInteger());
    res->setPrimaryResidueFeatures(inputs[2].getObject());
    res->setAccumulator(inputs[3].getObject());
    res->setGlobalFeatures(inputs[4].getObject());
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
