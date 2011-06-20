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
  globalType,
  residueType,
  residuePairType,
  disulfideBondType,
  cysteinBondingStateType
};

inline ProteinPerceptionType typeOfProteinPerception(ProteinTarget target)
{
  switch (target) {
    case cbpTarget: return globalType;
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
    case cbsTarget: return cysteinBondingStateType;
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

extern ClassPtr numericalProteinPrimaryFeaturesClass(TypePtr first, TypePtr second);

class NumericalProteinPrimaryFeatures : public ProteinPrimaryPerception
{
public:
  NumericalProteinPrimaryFeatures(TypePtr first, TypePtr second)
    : ProteinPrimaryPerception(numericalProteinPrimaryFeaturesClass(first, second)) {}
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
    firstTemplate = inputVariables[3]->getType()->getTemplateArgument(0)->getTemplateArgument(0);
    secondTemplate = inputVariables[5]->getType()->getTemplateArgument(0);
    return numericalProteinPrimaryFeaturesClass(firstTemplate, secondTemplate);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    NumericalProteinPrimaryFeaturesPtr res = new NumericalProteinPrimaryFeatures(firstTemplate, secondTemplate);
    res->setProtein(inputs[0].getObject());
    res->setLenght((size_t)inputs[1].getInteger());
    res->setNumCysteins((size_t)inputs[2].getInteger());
    res->setPrimaryResidueFeatures(inputs[3].getObject());
    res->setAccumulator(inputs[4].getObject());
    res->setGlobalFeatures(inputs[5].getObject());
    return res;
  }

private:
  TypePtr firstTemplate;
  TypePtr secondTemplate;
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

class GetCysteinIndexFromProteinIndex : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)proteinClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return positiveIntegerType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>();
    if (!protein)
      return Variable::missingValue(positiveIntegerType);
    size_t position = inputs[1].getInteger();
    jassert(position < protein->getLength());
    int cysteinIndex = protein->getCysteinInvIndices()[position];
    if (cysteinIndex < 0)
      return Variable::missingValue(positiveIntegerType);
    return Variable((size_t)cysteinIndex, positiveIntegerType);
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

// Cystein Matrix[Probability], Cystein Index -> Double Entropy (on row or column)
class ComputeCysteinEntropy : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)matrixClass(probabilityType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!inputs[1].exists())
      return Variable::missingValue(doubleType);
    const MatrixPtr& matrix = inputs[0].getObjectAndCast<Matrix>();
    if (!matrix)
      return Variable::missingValue(doubleType);

    const size_t position = inputs[1].getInteger();
    const size_t numRows = matrix->getNumRows();
    double entropy = 0.0;
    for (size_t i = 0; i < numRows; ++i)
    {
      Variable v = matrix->getElement(position, i);
      if (!v.exists() || v.getDouble() < 1e-06)
        continue;
      const double value = v.getDouble();
      entropy -= value * log2(value);
    }
    return Variable(entropy, doubleType);
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
      subInputs[0] = Variable(cysteinIndices[i], positiveIntegerType);
      for (size_t j = i; j < n; ++j)
      {
        subInputs[1] = Variable(cysteinIndices[j], positiveIntegerType);
        res->setElement(i, j, elementGeneratorFunction->compute(context, subInputs));
      }
    }
    return res;
  }

protected:
  friend class CreateDisulfideSymmetricMatrixFunctionClass;

  FunctionPtr elementGeneratorFunction;
};

class CreateCysteinBondingStateVectorFunction : public Function
{
public:
  CreateCysteinBondingStateVectorFunction(FunctionPtr elementGeneratorFunction = FunctionPtr())
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
    if (!elementGeneratorFunction->initialize(context, inputVars))
      return TypePtr();

    VariableSignaturePtr elementsSignature = elementGeneratorFunction->getOutputVariable();
    outputName = elementsSignature->getName() + T("CysteinBondingStateVector");
    outputShortName = elementsSignature->getShortName() + T("cysVect");
    return vectorClass(elementsSignature->getType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();
    ProteinPtr protein = inputs[0].getObjectAndCast<Protein>();

    const std::vector<size_t> cysteinIndices = protein->getCysteinIndices();
    const size_t n = cysteinIndices.size();

    VectorPtr res = vector(elementGeneratorFunction->getOutputType(), n);
    std::vector<Variable> subInputs(numInputs);
    for (size_t i = 1; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i];

    for (size_t i = 0; i < n; ++i)
    {
      subInputs[0] = Variable(cysteinIndices[i], positiveIntegerType);
      res->setElement(i, elementGeneratorFunction->compute(context, subInputs));
    }
    return res;
  }

protected:
  friend class CreateCysteinBondingStateVectorFunctionClass;

  FunctionPtr elementGeneratorFunction;
};

class CreateCysteinSeparationProfil : public Function
{
public:
  CreateCysteinSeparationProfil(bool normalize = true)
    : normalize(normalize) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)proteinClass;}

  virtual String getOutputPostFix() const
    {return T("CysSepProfil");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return vectorClass(doubleType);}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    ProteinPtr protein = inputs[0].getObjectAndCast<Protein>(context);
    jassert(protein);
    size_t position = inputs[1].getInteger();

    const std::vector<size_t>& cysteinIndices = protein->getCysteinIndices();
    const size_t n = cysteinIndices.size();
    if (!n)
      return VectorPtr();
    
    const size_t z = cysteinIndices[n-1] - cysteinIndices[0];
    VectorPtr res = vector(doubleType, n);

    for (size_t i = 0; i < n; ++i)
      res->setElement(i, Variable((double)abs((int)cysteinIndices[i] - (int)position) / (z ? (double)z : 1.f), doubleType));

    return res;
  }
  
protected:
  friend class CreateCysteinSeparationProfilClass;

  bool normalize;
};

class CysteinBondingStateRatio : public SimpleUnaryFunction
{
public:
  CysteinBondingStateRatio() : SimpleUnaryFunction(doubleVectorClass(enumValueType, probabilityType), probabilityType, T("BondedRatio")) {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const DoubleVectorPtr& vector = input.getObjectAndCast<DoubleVector>(context);
    if (!vector)
      return probability(0.f);
    
    const size_t n = vector->getNumElements();
    if (!n)
      return probability(0.f);
    
    double res = 0.0;
    for (size_t i = 0; i < n; i++)
      res += vector->getElement(i).getDouble();
    return probability(res / (double)n);
  }
};

class GreedyDisulfideBondSumOfRow : public Function
{
public:
  GreedyDisulfideBondSumOfRow(size_t minimumDistanceFromDiagonal = 1)
    : minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual String getOutputPostFix() const
    {return T("GreedySumOfRow");}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const SymmetricMatrixPtr& matrix = inputs[0].getObjectAndCast<SymmetricMatrix>(context);
    if (!matrix)
      return Variable(0.f, getOutputType());

    const size_t index = inputs[1].getInteger();
    
    const size_t dimension = matrix->getDimension();
    
    double sum = 0.f;
    if (index >= minimumDistanceFromDiagonal)
      for (size_t i = 0; i <= index - minimumDistanceFromDiagonal; ++i)
        sum += matrix->getElement(index, i).getDouble();
    for (size_t i = index + minimumDistanceFromDiagonal; i < dimension; ++i)
      sum += matrix->getElement(index, i).getDouble();

    jassert(sum <= 1 + 10e-6);
    return probability(sum);
  }
  
protected:
  friend class GreedyDisulfideBondSumOfRowClass;

  size_t minimumDistanceFromDiagonal;
};

class GreedyDisulfideBondRatio : public SimpleUnaryFunction
{
public:
  GreedyDisulfideBondRatio(size_t minimumDistanceFromDiagonal = 1)
    : SimpleUnaryFunction(symmetricMatrixClass(probabilityType), probabilityType, T("GreedyRatio"))
    , minimumDistanceFromDiagonal(minimumDistanceFromDiagonal) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const SymmetricMatrixPtr& matrix = input.getObjectAndCast<SymmetricMatrix>(context);
    if (!matrix)
      return probability(0.f);
    
    const size_t dimension = matrix->getDimension();
    if (dimension <= minimumDistanceFromDiagonal)
      return probability(0.f);
    
    double sum = 0.f;
    for (size_t i = 0; i < dimension - minimumDistanceFromDiagonal; ++i)
      for (size_t j = i + minimumDistanceFromDiagonal; j < dimension; ++j)
        sum += matrix->getElement(i, j).getDouble();

    jassert(sum <= (double)(dimension / 2) + 10e-6);
    return probability(sum / (double)(dimension / 2));
  }
  
protected:
  friend class GreedyDisulfideBondRatioClass;

  size_t minimumDistanceFromDiagonal;
};

// DoubleVector<T>, PositiveInteger -> T
class GetDoubleVectorValueFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)doubleVectorClass(enumValueType, doubleType);}

  virtual String getOutputPostFix() const
    {return T("Element");}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return Container::getTemplateParameter(inputVariables[0]->getType());}

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& vector = inputs[0].getObjectAndCast<Container>();
    if (vector)
    {
      int index = inputs[1].getInteger();
      if (index >= 0 && index < (int)vector->getNumElements())
        return vector->getElement((size_t)index);
    }

    return Variable(0.f, getOutputType());
  }
};

class GetSupervisionFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return anyType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return inputVariables[1]->getType();}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
    {return inputs[1];}
};

class SumDoubleVectorFunction : public Function
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return doubleVectorClass(enumValueType, doubleType);}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    TypePtr firstType = inputVariables[0]->getType()->getTemplateArgument(0);
    TypePtr secondType = inputVariables[0]->getType()->getTemplateArgument(0);
    
    if (firstType != secondType)
    {
      jassertfalse;
      return EnumerationPtr();
    }

    return (TypePtr)denseDoubleVectorClass(firstType, doubleType);
  }
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const DoubleVectorPtr& v1 = inputs[0].getObjectAndCast<DoubleVector>();
    const DoubleVectorPtr& v2 = inputs[1].getObjectAndCast<DoubleVector>();
    if (!v1 || !v2)
      return Variable::missingValue(getOutputType());

    const size_t n = v1->getNumElements();
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType(), n);
    v1->addWeightedTo(res, 0, 1.f);
    v2->addWeightedTo(res, 0, 1.f);
    
    return res;
  }
};

class GetPairBondingStateProbabilities : public FeatureGenerator
{
public:
  GetPairBondingStateProbabilities(bool useSpecialFeatures = false, size_t windowHalfSize = 0)
    : useSpecialFeatures(useSpecialFeatures), windowHalfSize(windowHalfSize) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)doubleVectorClass(enumValueType, probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();

    for (int i = -windowHalfSize; i <= (int)windowHalfSize; ++i)
    {
      res->addElement(context, T("p1[0]+p2[" + String(i) + "]"));
      res->addElement(context, T("p1[0]xp2[" + String(i) + "]"));
      if (i)
      {
        res->addElement(context, T("p1[" + String(i) + "]+p2[0]"));
        res->addElement(context, T("p1[" + String(i) + "]xp2[0]"));
      }
    }
    
    if (useSpecialFeatures)
      for (int i = -windowHalfSize; i <= (int)windowHalfSize; ++i)
      {
        res->addElement(context, T("p2[" + String(i) + "]"));
        res->addElement(context, T("1-p2[" + String(i) + "]"));

        res->addElement(context, T("p1[" + String(i) + "]"));
        res->addElement(context, T("1-p1[" + String(i) + "]"));
      }

    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const DoubleVectorPtr& values = inputs[0].getObjectAndCast<DoubleVector>();
    if (!values)
      return;
    const size_t n = values->getNumElements();

    const size_t firstIndex = inputs[1].getInteger();
    const size_t secondIndex = inputs[2].getInteger();

    const double pFirst = values->getElement(firstIndex).getDouble();
    const double pSecond = values->getElement(secondIndex).getDouble();

    size_t index = 0;
    for (int i = -windowHalfSize; i < (int)windowHalfSize + 1; ++i)
    {
      size_t thirdIndex = secondIndex + i;
      if (thirdIndex >= 0 && thirdIndex < n)
      {
        const double value = values->getElement(thirdIndex).getDouble();
        callback.sense(index, pFirst + value);
        callback.sense(index + 1, pFirst * value);
      }
      index += 2;
      if (i)
      {
        thirdIndex = firstIndex + i;
        if (thirdIndex >= 0 && thirdIndex < n)
        {
          const double value = values->getElement(thirdIndex).getDouble();
          callback.sense(index, pSecond + value);
          callback.sense(index + 1, pSecond * value);
        }
        index += 2;
      }
    }
    
    if (useSpecialFeatures)
      for (int i = -windowHalfSize; i < (int)windowHalfSize + 1; ++i)
      {
        size_t thirdIndex = secondIndex + i;
        if (thirdIndex >= 0 && thirdIndex < n)
        {
          const double value = values->getElement(thirdIndex).getDouble();
          callback.sense(index, value);
          callback.sense(index + 1, 1 - value);
        }
        index += 2;
        thirdIndex = firstIndex + i;
        if (thirdIndex >= 0 && thirdIndex < n)
        {
          const double value = values->getElement(thirdIndex).getDouble();
          callback.sense(index, value);
          callback.sense(index + 1, 1 - value);
        }
        index += 2;
      }
  }

protected:
  friend class GetPairBondingStateProbabilitiesClass;

  bool useSpecialFeatures;
  size_t windowHalfSize;
};

class GetBondingStateProbabilities : public FeatureGenerator
{
public:
  GetBondingStateProbabilities(bool useSpecialFeatures = false)
    : useSpecialFeatures(useSpecialFeatures) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)doubleVectorClass(enumValueType, probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("p"));
    if (useSpecialFeatures)
      res->addElement(context, T("1-p"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const DoubleVectorPtr& values = inputs[0].getObjectAndCast<DoubleVector>();
    if (!values)
      return;
    const double p = values->getElement(inputs[1].getInteger()).getDouble();
    callback.sense(0, p);
    if (useSpecialFeatures)
      callback.sense(1, 1 - p);
  }

protected:
  friend class GetBondingStateProbabilitiesClass;

  bool useSpecialFeatures;
};

class GetDisulfideBondProbability : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("p1,2"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const SymmetricMatrixPtr& matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    const double p = matrix->getElement(inputs[1].getInteger(), inputs[2].getInteger()).getDouble();
    callback.sense(0, p);
  }
};

class ProteinLengthFeatureGenerator : public FeatureGenerator
{
public:
  ProteinLengthFeatureGenerator(size_t maxLength, size_t stepSize, bool lazy = false) 
    : FeatureGenerator(lazy), maxLength(maxLength), stepSize(stepSize)
    {jassert(stepSize && maxLength);}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return proteinClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("ProteinLengthFeatures"));

    const size_t numFeatures = maxLength / stepSize;
    for (size_t i = 0; i < numFeatures; ++i)
      res->addElement(context, T("[") + String((int)(i * stepSize)) + T(";") + String((int)((i + 1) * stepSize)) + T("["));
    res->addElement(context, T("[") + String((int)(numFeatures * stepSize)) + T(";+inf"));

    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);

    const size_t numFeatures = maxLength / stepSize;
    size_t index = protein->getLength() / stepSize;
    if (index > numFeatures)
      index = numFeatures;

    callback.sense(index, 1.0);
  }

protected:
  size_t maxLength;
  size_t stepSize;
};

class ProteinLengthNormalized : public SimpleUnaryFunction
{
public:
  ProteinLengthNormalized(size_t maxLength)
    : SimpleUnaryFunction(proteinClass, probabilityType, T("ProteinLengthNormalized")), maxLength(maxLength) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    
    const size_t length = protein->getLength();
    if (length > maxLength)
    {
      jassertfalse;
      return probability(1.f);
    }
    return probability(length / (double)maxLength);
  }

protected:
  size_t maxLength;
};

class NumCysteinsFeatureGenerator : public FeatureGenerator
{
public:
  NumCysteinsFeatureGenerator(bool hardDiscretization = true, bool lazy = false) 
    : FeatureGenerator(lazy), hardDiscretization(hardDiscretization) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return proteinClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration(T("NumCysteinsFeatures"));

    if (!hardDiscretization)
    {
      res->addElement(context, T("value"));
      return res;
    }

    res->addElement(context, T("1-"));
    res->addElement(context, T("2"));
    res->addElement(context, T("3"));
    res->addElement(context, T("4"));
    res->addElement(context, T("5"));
    res->addElement(context, T("[6;10["));
    res->addElement(context, T("[10;15["));
    res->addElement(context, T("[15;20["));
    res->addElement(context, T("[20;25["));
    res->addElement(context, T("25+"));
    
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);

    const size_t numCysteins = protein->getCysteinIndices().size();

    if (!hardDiscretization)
    {
      callback.sense(0, (double)numCysteins);
      return;
    }

    size_t index = 9; // 25+
    if (numCysteins <= 1)
      index = 0;
    else if (numCysteins == 2)
      index = 1;
    else if (numCysteins == 3)
      index = 2;
    else if (numCysteins == 4)
      index = 3;
    else if (numCysteins == 5)
      index = 4;
    else if (numCysteins < 10)
      index = 5;
    else if (numCysteins < 15)
      index = 6;
    else if (numCysteins < 20)
      index = 7;
    else if (numCysteins < 25)
      index = 8;

    callback.sense(index, 1.0);
  }
  
protected:
  bool hardDiscretization;
};

class IsNumCysteinPair : public SimpleUnaryFunction
{
public:
  IsNumCysteinPair()
    : SimpleUnaryFunction(proteinClass, probabilityType, T("IsNumCysteinPair")) {}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const ProteinPtr& protein = input.getObjectAndCast<Protein>(context);
    jassert(protein);
    
    const size_t numCysteins = protein->getCysteinIndices().size();
    return probability((numCysteins + 1) % 2);
  }

protected:
  size_t maxLength;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
