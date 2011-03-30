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

inline ProteinPerceptionType typeOfProteinPerception(ProteinTarget target)
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

class ProteinPrimaryPerception : public Object
{
public:
  void setProtein(ProteinPtr protein)
    {this->protein = protein;}

  void setLenght(size_t length)
    {this->length = length;}

  void setNumCysteins(size_t numCysteins)
    {this->numCysteins = numCysteins;}

protected:
  friend class ProteinPrimaryPerceptionClass;

  ProteinPtr protein;
  size_t length;
  size_t numCysteins;
  
  ProteinPrimaryPerception(TypePtr type)
    : Object(type) {}
  ProteinPrimaryPerception() {}
};

extern ClassPtr numericalProteinPrimaryFeaturesClass(TypePtr type);

class NumericalProteinPrimaryFeatures : public ProteinPrimaryPerception
{
public:
  NumericalProteinPrimaryFeatures(TypePtr type)
    : ProteinPrimaryPerception(numericalProteinPrimaryFeaturesClass(type)) {}
  NumericalProteinPrimaryFeatures() {}
  
  void setPrimaryResidueFeatures(VectorPtr features)
    {this->primaryResidueFeatures = features;}

  void setAccumulator(ContainerPtr accumulator)
    {this->accumulator = accumulator;}

  void setGlobalFeatures(DoubleVectorPtr globalFeatures)
    {this->globalFeatures = globalFeatures;}
  
protected:
  friend class NumericalProteinPrimaryFeaturesClass;

  VectorPtr primaryResidueFeatures;
  ContainerPtr accumulator;
  DoubleVectorPtr globalFeatures;
};

typedef ReferenceCountedObjectPtr<NumericalProteinPrimaryFeatures> NumericalProteinPrimaryFeaturesPtr;

class CreateProteinPerceptionFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 6;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
    switch (index)
    {
      case 0: return proteinClass;
      case 1: return positiveIntegerType;
      case 2: return positiveIntegerType;
      case 3: return vectorClass(doubleVectorClass());
      case 4: return containerClass(doubleVectorClass());
      case 5: return doubleVectorClass();
      default:
        jassertfalse;
        return TypePtr();
    }
  }

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr res = inputVariables[4]->getType()->getTemplateArgument(0);
    return numericalProteinPrimaryFeaturesClass(res);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    NumericalProteinPrimaryFeaturesPtr res = new NumericalProteinPrimaryFeatures(getOutputType()->getTemplateArgument(0));
    res->setProtein(inputs[0].getObject());
    res->setLenght((size_t)inputs[1].getInteger());
    res->setNumCysteins((size_t)inputs[2].getInteger());
    res->setPrimaryResidueFeatures(inputs[3].getObject());
    res->setAccumulator(inputs[4].getObject());
    res->setGlobalFeatures(inputs[5].getObject());
    return res;
  }
};

/*
** ProteinLengthFunction
*/
class ProteinLengthFunction : public SimpleUnaryFunction
{
public:
  ProteinLengthFunction() : SimpleUnaryFunction(proteinClass, positiveIntegerType, T("Length")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    if (protein)
      return Variable(protein->getLength(), positiveIntegerType);
    return Variable::missingValue(positiveIntegerType);
  }
};

class NumCysteinsFunction : public SimpleUnaryFunction
{
public:
  NumCysteinsFunction() : SimpleUnaryFunction(proteinClass, positiveIntegerType, T("NumCysteins")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    if (protein)
      return Variable(protein->getCysteinIndices().size(), positiveIntegerType);
    return Variable::missingValue(positiveIntegerType);
  }
};

class NormalizeDisulfideBondFunction : public SimpleUnaryFunction
{
public:
  NormalizeDisulfideBondFunction() : SimpleUnaryFunction(symmetricMatrixClass(probabilityType), matrixClass(probabilityType), T("NormalizeDsb")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const SymmetricMatrixPtr& disulfideBonds = input.getObjectAndCast<SymmetricMatrix>(context);
    if (!disulfideBonds)
      return Variable::missingValue(matrixClass(probabilityType));

    const size_t dimension = disulfideBonds->getDimension();
    if (dimension <= 1)
      return Variable::missingValue(matrixClass(probabilityType));

    std::vector<double> z(dimension, 0.0);
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = 0; j < dimension; ++j)
        if (i != j)
          z[i] += disulfideBonds->getElement(i, j).getDouble();

    MatrixPtr res = matrix(probabilityType, dimension, dimension);
    for (size_t i = 0; i < dimension; ++i)
      for (size_t j = 0; j < dimension; ++j)
        if (i != j)
          res->setElement(i, j, z[i] == 0.0 ? probability(0.0) : disulfideBonds->getElement(i, j).getDouble() / z[i]);

    return res;
  }
};

class ApplyFeatureGeneratorOnCytein : public FeatureGenerator
{
public:
  ApplyFeatureGeneratorOnCytein(FeatureGeneratorPtr decorated)
    : FeatureGenerator(decorated->isLazy()), decorated(decorated) {}
  
  virtual size_t getMinimumNumRequiredInputs() const
    {return 3;}
  
  virtual size_t getMaximumNumRequiredInputs() const
    {return /*(size_t)-1*/4;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
    if (index == 0)
      return proteinClass;
    if (index < 3)
      return positiveIntegerType;
    return anyType;
  }

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> subInputs(3);
    subInputs[0] = inputVariables[3];
    subInputs[1] = inputVariables[1];
    subInputs[2] = inputVariables[2];
    return decorated->initializeFeatures(context, subInputs, elementsType, outputName, outputShortName);
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>();
    if (!protein)
      return;
    size_t firstPosition = inputs[1].getInteger();
    size_t secondPosition = inputs[2].getInteger();
    jassert(firstPosition < protein->getLength() && secondPosition < protein->getLength());
    const std::vector<int>& cysteinInvIndices = protein->getCysteinInvIndices();
    if (cysteinInvIndices[firstPosition] < 0 || cysteinInvIndices[secondPosition] < 0)
      return;

    const size_t numInputs = getNumInputs();
    std::vector<Variable> subInputs(numInputs - 1);
    subInputs[0] = inputs[3]; // For MatrixWindonFeatureGenerator, the first input must be the Matrix
    subInputs[1] = cysteinInvIndices[firstPosition];
    subInputs[2] = cysteinInvIndices[secondPosition];
    decorated->computeFeatures(&subInputs[0], callback);
  }

protected:
  FeatureGeneratorPtr decorated;
};

class GetCysteinProbabilityFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 4;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)proteinClass : (index == 1 ? (TypePtr)symmetricMatrixClass(probabilityType) : positiveIntegerType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>(context);
    const SymmetricMatrixPtr& matrix = inputs[1].getObjectAndCast<SymmetricMatrix>(context);
    size_t firstPosition = inputs[2].getInteger();
    size_t secondPosition = inputs[3].getInteger();

    const std::vector<int>& cysteinInvIndices = protein->getCysteinInvIndices();
    int firstCysteinPosition = cysteinInvIndices[firstPosition];
    int secondCysteinPosition = cysteinInvIndices[secondPosition];
    if (matrix && firstCysteinPosition != -1 && secondCysteinPosition != -1)
      return matrix->getElement(firstCysteinPosition, secondCysteinPosition);
    return Variable::missingValue(probabilityType);
  }
};

class SubtractFunction : public Function
{
public:
  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}
  
  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return sumType(doubleType, integerType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();
    double res = inputs[0].isDouble() ? inputs[0].getDouble() : (double)inputs[0].getInteger();
    for (size_t i = 1; i < numInputs; ++i)
      res -= inputs[i].isDouble() ? inputs[i].getDouble() : (double)inputs[i].getInteger();
    return Variable(res, doubleType);
  }
};

// generates a symmetric matrix M(i, j) = f(i, j, x) where i,j in [0,n[ from input n,x
class CreateDisulfideSymmetricMatrixFunction : public Function
{
public:
  CreateDisulfideSymmetricMatrixFunction(FunctionPtr elementGeneratorFunction = FunctionPtr())
    : elementGeneratorFunction(elementGeneratorFunction) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return (index == 0) ? (TypePtr)proteinClass : elementGeneratorFunction->getRequiredInputType(index + 1, numInputs);}

  virtual String getOutputPostFix() const
    {return T("Generated");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    std::vector<VariableSignaturePtr> inputVars = inputVariables;
    inputVars[0] = new VariableSignature(positiveIntegerType, T("Position"));
    inputVars.insert(inputVars.begin(), inputVars.front());
    if (!elementGeneratorFunction->initialize(context, inputVars))
      return TypePtr();

    VariableSignaturePtr elementsSignature = elementGeneratorFunction->getOutputVariable();
    outputName = elementsSignature->getName() + T("DisulfideSymmetricMatrix");
    outputShortName = elementsSignature->getShortName() + T("sm");
    return symmetricMatrixClass(elementsSignature->getType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();
    ProteinPtr protein = inputs[0].getObjectAndCast<Protein>();

    const std::vector<size_t> cysteinIndices = protein->getCysteinIndices();
    const size_t n = cysteinIndices.size();

    SymmetricMatrixPtr res = symmetricMatrix(elementGeneratorFunction->getOutputType(), n);
    std::vector<Variable> subInputs(numInputs + 1);
    for (size_t i = 2; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i - 1];

    for (size_t i = 0; i < n; ++i)
    {
      subInputs[0] = Variable(cysteinIndices[i]);
      for (size_t j = i; j < n; ++j)
      {
        subInputs[1] = Variable(cysteinIndices[j]);
        res->setElement(i, j, elementGeneratorFunction->compute(context, subInputs));
      }
    }
    return res;
  }

protected:
  friend class CreateDisulfideSymmetricMatrixFunctionClass;

  FunctionPtr elementGeneratorFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
