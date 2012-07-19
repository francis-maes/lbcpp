/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.h            | Protein Perception              |
| Author  : Julien Becker                  |                                 |
| Started : 17/02/2011 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_PERCEPTION_H_
# define LBCPP_PROTEIN_PERCEPTION_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

enum ProteinPerceptionType
{
  noProteinPerceptionType,
  globalType,
  residueType,
  residuePairType,
  disulfideSymmetricBondType,
  oxidizedDisulfideSymmetricBondType,
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
    case fdsbTarget: return disulfideBondType;
    case dsbTarget: return disulfideSymmetricBondType;
    case odsbTarget: return oxidizedDisulfideSymmetricBondType;
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

  ProteinPtr getProtein() const
    {return protein;}

protected:
  friend class ProteinPrimaryPerceptionClass;

  ProteinPtr protein;
  size_t length;
  size_t numCysteins;
  
  ProteinPrimaryPerception(TypePtr type)
    : Object(type) {}
  ProteinPrimaryPerception() {}
};

typedef ReferenceCountedObjectPtr<ProteinPrimaryPerception> ProteinPrimaryPerceptionPtr;

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
    jassert(protein);
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
  CreateDisulfideSymmetricMatrixFunction(FunctionPtr elementGeneratorFunction, double oxidizedCysteineThreshold)
    : elementGeneratorFunction(elementGeneratorFunction), oxidizedCysteineThreshold(oxidizedCysteineThreshold) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)proteinClass : elementGeneratorFunction->getRequiredInputType(index + 1, numInputs);}

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

    const std::vector<size_t>& cysteinIndices = protein->getCysteinIndices();
    const size_t n = cysteinIndices.size();

    std::vector<bool> oxidizedCysteines;
    protein->getCysteinBondingStates(oxidizedCysteineThreshold, oxidizedCysteines);

    SymmetricMatrixPtr res = symmetricMatrix(elementGeneratorFunction->getOutputType(), n);
    std::vector<Variable> subInputs(numInputs + 1);
    for (size_t i = 2; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i - 1];

    for (size_t i = 0; i < n; ++i)
    {
      if (!oxidizedCysteines[i])
        continue;

      subInputs[0] = Variable(cysteinIndices[i], positiveIntegerType);
      for (size_t j = i; j < n; ++j)
      {
        if (!oxidizedCysteines[j])
          continue;

        subInputs[1] = Variable(cysteinIndices[j], positiveIntegerType);
        res->setElement(i, j, elementGeneratorFunction->compute(context, subInputs));
      }
    }
    return res;
  }

protected:
  friend class CreateDisulfideSymmetricMatrixFunctionClass;

  FunctionPtr elementGeneratorFunction;
  double oxidizedCysteineThreshold;

  CreateDisulfideSymmetricMatrixFunction() {}
};

// generates a matrix M(i, j) = f(i, j, x) where i,j in [0,n[ from input n,x
class CreateDisulfideMatrixFunction : public Function
{
public:
  CreateDisulfideMatrixFunction(FunctionPtr elementGeneratorFunction = FunctionPtr())
    : elementGeneratorFunction(elementGeneratorFunction) {}

  virtual size_t getMinimumNumRequiredInputs() const
    {return 1;}

  virtual size_t getMaximumNumRequiredInputs() const
    {return (size_t)-1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)proteinClass : elementGeneratorFunction->getRequiredInputType(index + 1, numInputs);}

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
    outputName = elementsSignature->getName() + T("DisulfideMatrix");
    outputShortName = elementsSignature->getShortName() + T("sm");
    return matrixClass(elementsSignature->getType());
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    size_t numInputs = getNumInputs();
    ProteinPtr protein = inputs[0].getObjectAndCast<Protein>();

    const std::vector<size_t> cysteinIndices = protein->getCysteinIndices();
    const size_t n = cysteinIndices.size();

    MatrixPtr res = matrix(elementGeneratorFunction->getOutputType(), n, n);
    std::vector<Variable> subInputs(numInputs + 1);
    for (size_t i = 2; i < subInputs.size(); ++i)
      subInputs[i] = inputs[i - 1];

    for (size_t i = 0; i < n; ++i)
    {
      subInputs[0] = Variable(cysteinIndices[i], positiveIntegerType);
      for (size_t j = 0; j < n; ++j)
      {
        if (i == j)
        {
          res->setElement(i, j, Variable::missingValue(elementGeneratorFunction->getOutputType()));
          continue;
        }
        subInputs[1] = Variable(cysteinIndices[j], positiveIntegerType);
        res->setElement(i, j, elementGeneratorFunction->compute(context, subInputs));
      }
    }
    return res;
  }

protected:
  friend class CreateDisulfideMatrixFunctionClass;

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
  CreateCysteinSeparationProfil(bool normalizeWithProteinLength = false, double oxidizedCysteineThreshold = 0.f)
    : normalizeWithProteinLength(normalizeWithProteinLength), oxidizedCysteineThreshold(oxidizedCysteineThreshold) {}

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

    std::vector<bool> oxidizedCysteines;
    size_t numOxidizedCysteines = protein->getCysteinBondingStates(oxidizedCysteineThreshold, oxidizedCysteines);

    if (!numOxidizedCysteines)
      return Variable::missingValue(getOutputType());
    
    size_t firstCysteineIndex = (size_t)-1;
    size_t lastCysteineIndex = (size_t)-1;
    for (size_t i = 0; i < n; ++i)
    {
      if (!oxidizedCysteines[i])
        continue;
      if (firstCysteineIndex == (size_t)-1)
        firstCysteineIndex = cysteinIndices[i];
      lastCysteineIndex = cysteinIndices[i];
    }
    
    const size_t zFactor = normalizeWithProteinLength ? protein->getLength() -1 : lastCysteineIndex - firstCysteineIndex;
    VectorPtr res = vector(doubleType, numOxidizedCysteines);
    for (size_t i = 0, index = 0; i < n; ++i)
      if (oxidizedCysteines[i])
        res->setElement(index++, Variable((double)abs((int)cysteinIndices[i] - (int)position) / (zFactor ? (double)zFactor : 1.f), doubleType));

    return res;
  }

protected:
  bool normalizeWithProteinLength;
  double oxidizedCysteineThreshold;
};

class CysteinSeparationProfilFeatureGenerator : public FeatureGenerator
{
public:
  CysteinSeparationProfilFeatureGenerator(size_t windowSize, bool normalizeWithProteinLength = false, double oxidizedCysteineThreshold = 0.f)
    : windowSize(windowSize), normalizeWithProteinLength(normalizeWithProteinLength), oxidizedCysteineThreshold(oxidizedCysteineThreshold) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)proteinClass;}

  virtual String getOutputPostFix() const
    {return T("CysSepProfil");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    const int startIndex = -(int)windowSize / 2;
    for (size_t i = 0; i < windowSize; ++i)
      res->addElement(context, T("[") + String((int)i + startIndex) + ("]"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    ProteinPtr protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);
    size_t position = inputs[1].getInteger();

    const std::vector<size_t>& cysteinIndices = protein->getCysteinIndices();
    
    std::vector<bool> oxidizedCysteines;
    size_t numOxidizedCysteines = protein->getCysteinBondingStates(oxidizedCysteineThreshold, oxidizedCysteines);

    if (!numOxidizedCysteines)
      return;
    
    std::vector<size_t> oxidizedCysteineIndices;
    oxidizedCysteineIndices.reserve(numOxidizedCysteines);
    for (size_t i = 0; i < cysteinIndices.size(); ++i)
      if (oxidizedCysteines[i])
        oxidizedCysteineIndices.push_back(cysteinIndices[i]);

    size_t zFactor = normalizeWithProteinLength ? protein->getLength() -1 : oxidizedCysteineIndices.back() - oxidizedCysteineIndices.front();
    if (zFactor == 0)
      zFactor = 1;

    size_t index = numOxidizedCysteines;
    for (size_t i = 0; i < numOxidizedCysteines; ++i)
      if (position <= oxidizedCysteineIndices[i])
      {
        index = i;
        break;
      }

    const int startCysteinIndex = index - windowSize / 2;
    for (size_t i = (startCysteinIndex < 0) ? -startCysteinIndex : 0; i < windowSize && startCysteinIndex + i < numOxidizedCysteines; ++i)
      callback.sense(i, (double)abs((int)oxidizedCysteineIndices[startCysteinIndex + i] - (int)position) / (double)zFactor);
  }

protected:
  size_t windowSize;
  bool normalizeWithProteinLength;
  double oxidizedCysteineThreshold;
};
  

class DisuflideSeparationProfilFeatureGenerator : public FeatureGenerator
{
public:
  DisuflideSeparationProfilFeatureGenerator(size_t windowSize, bool normalize)
    : windowSize(windowSize), normalize(normalize) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(doubleType);}

  virtual String getOutputPostFix() const
    {return T("DsbSepProfil");}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    const int startIndex = -(int)windowSize / 2;
    for (size_t i = 0; i < windowSize; ++i)
      res->addElement(context, T("[") + String((int)i + startIndex) + ("]"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    size_t index = inputs[1].getInteger();

    const size_t n = matrix->getDimension();
    if (n <= 1)
      return;

    double zFactor = 1.0;
    if (normalize)
    {
      zFactor = 0.0;
      for (size_t i = 0; i < n; ++i)
        if (i != index)
          zFactor += matrix->getElement(index, i).getDouble();
      jassert(zFactor > 1e-6);
    }

    const int startCysteinIndex = index - windowSize / 2;
    for (size_t i = (startCysteinIndex < 0) ? -startCysteinIndex : 0;
         i < windowSize && startCysteinIndex + i < n; ++i)
      if (i != index)
        callback.sense(i, matrix->getElement(index, startCysteinIndex + i).getDouble() / zFactor);
  }

protected:
  size_t windowSize;
  bool normalize;
};

class DisulfideInfoFeatureGenerator : public FeatureGenerator
{
public:
  DisulfideInfoFeatureGenerator(bool useProbability, bool useNormProbability)
    : useProbability(useProbability), useNormProbability(useNormProbability) {}

  virtual size_t getNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(doubleType);}
  
  virtual String getOutputPostFix() const
    {return T("dsb");}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    if (useProbability)
      res->addElement(context, T("[DSB(i,j)]"));
    if (useNormProbability)
    {
      res->addElement(context, T("[DSB(i,j)/Sum_i]"));
      res->addElement(context, T("[DSB(i,j)/Sum_j]"));
    }
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    size_t firstIndex = inputs[1].getInteger();
    size_t secondIndex = inputs[1].getInteger();

    const size_t n = matrix->getDimension();
    if (n <= 1)
      return;

    jassert(firstIndex < n && secondIndex < n);

    size_t index = 0;
    if (useNormProbability)
      callback.sense(index++, matrix->getElement(firstIndex, secondIndex).getDouble());    
    
    if (useNormProbability)
    {
      double ziFactor = 0.0;
      for (size_t i = 0; i < n; ++i)
        ziFactor += matrix->getElement(firstIndex, i).getDouble();
      
      double zjFactor = 0.0;
      for (size_t i = 0; i < n; ++i)
        zjFactor += matrix->getElement(secondIndex, i).getDouble();
      
      callback.sense(index++, matrix->getElement(firstIndex, secondIndex).getDouble() / ziFactor);
      callback.sense(index++, matrix->getElement(firstIndex, secondIndex).getDouble() / zjFactor);
    }
  }

private:
  bool useProbability;
  bool useNormProbability;
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
class GetDoubleVectorValueFunction : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)doubleVectorClass(enumValueType, doubleType);}

  virtual String getOutputPostFix() const
    {return T("Element");}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("value"));
    return res;
  }

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const DoubleVectorPtr& vector = inputs[0].getObjectAndCast<Container>();
    if (vector)
    {
      int index = inputs[1].getInteger();
      if (index >= 0 && index < (int)vector->getNumElements())
        callback.sense(0, vector->getElement((size_t)index).getDouble());
    }
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

    for (int i = -(int)windowHalfSize; i <= (int)windowHalfSize; ++i)
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
      for (int i = -(int)windowHalfSize; i <= (int)windowHalfSize; ++i)
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
    for (int i = -(int)windowHalfSize; i < (int)windowHalfSize + 1; ++i)
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
      for (int i = -(int)windowHalfSize; i < (int)windowHalfSize + 1; ++i)
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

class IsNumCysteinPair : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return (TypePtr)proteinClass;}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("IsNumCysteinPair"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    const ProteinPtr& protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);
    
    const size_t numCysteins = protein->getCysteinIndices().size();
    callback.sense(0, (numCysteins + 1) % 2);
  }
};

class NormalizedCysteinPositionDifference : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)proteinClass;}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("ncpd"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    ProteinPtr protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);
    const size_t first = inputs[1].getInteger();
    const size_t second = inputs[2].getInteger();
    const size_t diff = (first < second) ? second - first : first - second;

    const std::vector<size_t>& cysteinIndices = protein->getCysteinIndices();
    callback.sense(0, diff / (double)(cysteinIndices[cysteinIndices.size()-1] - cysteinIndices[0]));
  }
};

class NormalizedCysteinIndexDifference : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)proteinClass;}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("ncid"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    ProteinPtr protein = inputs[0].getObjectAndCast<Protein>();
    jassert(protein);
    const size_t first = inputs[1].getInteger();
    const size_t second = inputs[2].getInteger();
    const size_t diff = (first < second) ? second - first : first - second;

    const std::vector<size_t>& cysteinIndices = protein->getCysteinIndices();
    callback.sense(0, diff / (double)cysteinIndices.size());
  }
};

class SimpleConnectivityPatternFeatureGenerator : public FeatureGenerator
{
public:
  SimpleConnectivityPatternFeatureGenerator(size_t maxConnection = 25, bool senseSumOfValues = false, double threshold = 0.5)
    : maxConnection(maxConnection), senseSumOfValues(senseSumOfValues), threshold(threshold) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    for (size_t i = 0; i < maxConnection; ++i)
      res->addElement(context, T("connectedTo[" + String((int)i) + "]"));
    res->addElement(context, T("connectedTo[" + String((int)maxConnection) + "+]"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    const size_t row = inputs[1].getInteger();
    const size_t dimension = matrix->getDimension();
    
    size_t numConnection = 0;
    double sumValues = 0.0;
    for (size_t i = 0; i < dimension; ++i)
    {
      if (i == row)
        continue;
      const double value = matrix->getElement(row, i).getDouble();
      if (value >= threshold)
      {
        ++numConnection;
        sumValues += value;
      }
    }
    
    callback.sense((numConnection >= maxConnection) ? maxConnection : numConnection
                  , senseSumOfValues ? sumValues / (double)numConnection : 1.0);
  }

protected:
  friend class SimpleConnectivityPatternFeatureGeneratorClass;
  
  size_t maxConnection;
  bool senseSumOfValues;
  double threshold;
};

class SymmetricMatrixRowStatisticsFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("min"));
    res->addElement(context, T("max"));
    res->addElement(context, T("mean"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    const size_t row = inputs[1].getInteger();
    const size_t dimension = matrix->getDimension();
    
    double min = DBL_MAX;
    double max = -DBL_MAX;
    double sum = 0.0;
    for (size_t i = 0; i < dimension; ++i)
    {
      if (i == row)
        continue;
      const double value = matrix->getElement(row, i).getDouble();
      sum += value;
      if (value < min)
        min = value;
      if (value > max)
        max = value;
    }
    callback.sense(0, min);
    callback.sense(1, max);
    callback.sense(2, sum / (double)(dimension - 1));
  }
};

class NumCommonNeighborsFeatureGenerator : public FeatureGenerator
{
public:
  NumCommonNeighborsFeatureGenerator(size_t maxNeighbors = 25)
    : maxNeighbors(maxNeighbors) {}
  
  virtual size_t getNumRequiredInputs() const
    {return 3;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("#commonNeigbors/") + String((int)maxNeighbors));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    const size_t firstIndex = inputs[1].getInteger();
    const size_t secondIndex = inputs[2].getInteger();
    const size_t dimension = matrix->getDimension();
    
    size_t numCommonNeighbors = 0;
    for (size_t i = 0; i < dimension; i++)
    {
      if (i != firstIndex && i != secondIndex
          && matrix->getElement(firstIndex, i).getDouble() > 0.5
          && matrix->getElement(secondIndex, i).getDouble() > 0.5)
        numCommonNeighbors++;
    }
    callback.sense(0, numCommonNeighbors / (double)maxNeighbors);
  }
protected:
  size_t maxNeighbors;
};
// |L(x) ^ L(y)| / |L(x) v L(y)|
class JaccardsCoefficientFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 3;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("value"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    const size_t firstIndex = inputs[1].getInteger();
    const size_t secondIndex = inputs[2].getInteger();
    const size_t dimension = matrix->getDimension();
    
    size_t numNeighbors = 0; // |L(x) v L(y)|
    for (size_t i = 0; i < dimension; i++)
    {
      if (i != firstIndex && i != secondIndex
          && (matrix->getElement(firstIndex, i).getDouble() > 0.5
              || matrix->getElement(secondIndex, i).getDouble() > 0.5))
        numNeighbors++;
    }
    
    size_t numCommonNeighbors = 0; // |L(x) ^ L(y)|
    for (size_t i = 0; i < dimension; i++)
    {
      if (i != firstIndex && i != secondIndex
          && matrix->getElement(firstIndex, i).getDouble() > 0.5
          && matrix->getElement(secondIndex, i).getDouble() > 0.5)
        numCommonNeighbors++;
    }
    if (numNeighbors)
      callback.sense(0, numCommonNeighbors / (double)numNeighbors);
  }
};

class AdamicAdarFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 3;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("value"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    const size_t firstIndex = inputs[1].getInteger();
    const size_t secondIndex = inputs[2].getInteger();
    const size_t dimension = matrix->getDimension();

    double sum = 0.0;
    for (size_t i = 0; i < dimension; i++)
    {
      if (i != firstIndex && i != secondIndex
          && matrix->getElement(firstIndex, i).getDouble() > 0.5
          && matrix->getElement(secondIndex, i).getDouble() > 0.5)
      {
        const size_t numNeighbors = getNumNeighbors(matrix, dimension, i);
        jassert(numNeighbors >= 2);
        sum += 1 / log2(numNeighbors); // log2 ou log10 ? Boh, It's not specified
      }
    }
    callback.sense(0, sum);
  }
  
protected:
  size_t getNumNeighbors(const SymmetricMatrixPtr& matrix, const size_t dimension, const size_t index) const
  {
    size_t res = 0;
    for (size_t i = 0; i < dimension; ++i)
      if (i != index && matrix->getElement(index, i).getDouble() > 0.5)
        ++res;
    return res;
  }
};

class PreferentialAttachementFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 3;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? positiveIntegerType : (TypePtr)symmetricMatrixClass(probabilityType);}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
  {
    DefaultEnumerationPtr res = new DefaultEnumeration();
    res->addElement(context, T("value"));
    return res;
  }
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    SymmetricMatrixPtr matrix = inputs[0].getObjectAndCast<SymmetricMatrix>();
    if (!matrix)
      return;
    const size_t firstIndex = inputs[1].getInteger();
    const size_t secondIndex = inputs[2].getInteger();
    const size_t dimension = matrix->getDimension();

    size_t attachement = getNumNeighbors(matrix, dimension, firstIndex)
                       * getNumNeighbors(matrix, dimension, secondIndex);
    callback.sense(0, attachement / (double)(dimension * dimension - 2));
  }

protected:
  size_t maxValue;

  size_t getNumNeighbors(const SymmetricMatrixPtr& matrix, const size_t dimension, const size_t index) const
  {
    size_t res = 0;
    for (size_t i = 0; i < dimension; ++i)
      if (i != index && matrix->getElement(index, i).getDouble() > 0.5)
        ++res;
    return res;
  }
};

class RelativeValueFeatureGenerator : public FeatureGenerator
{
public:
  RelativeValueFeatureGenerator(size_t incrementValue = 0)
    : incrementValue(incrementValue) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return positiveIntegerType;}
  
  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return singletonEnumeration;}
  
  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    if (!inputs[0].exists())
    {
      callback.sense(0, 0.0);
      return;
    }
    const size_t value = inputs[0].getInteger() + incrementValue;
    const size_t zFactor = inputs[1].getInteger();
    callback.sense(0, value / (double)zFactor);
  }

protected:
  size_t incrementValue;
};

class NormalizeDenseDoubleVector : public SimpleUnaryFunction
{
public:
  enum NormalizationType {none = 0, relative, mean, meanAndStandardDeviation};

  NormalizeDenseDoubleVector(NormalizationType type)
    : SimpleUnaryFunction(denseDoubleVectorClass(enumValueType, doubleType), vectorClass(denseDoubleVectorClass(singletonEnumeration, doubleType)), T("normDDV")),
      type(type)
  {}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    DenseDoubleVectorPtr ddv = input.getObjectAndCast<DenseDoubleVector>();
    if (!ddv)
      return Variable::missingValue(vectorClass(denseDoubleVectorClass(singletonEnumeration, doubleType)));

    const size_t n = ddv->getNumElements();
    std::vector<double> values(n, 0.f);
    for (size_t i = 0; i < n; ++i)
      if (ddv->getElement(i).exists())
        values[i] = ddv->getValue(i);

    ScalarVariableMeanAndVariance stat;
    for (size_t i = 0; i < values.size(); ++i)
      stat.push(values[i]);

    if (type == relative)
    {
      for (size_t i = 0; i < values.size(); ++i)
        values[i] /= stat.getSum();
    }
    else if (type == mean)
    {
      const double mean = stat.getMean();
      for (size_t i = 0; i < values.size(); ++i)
        values[i] -= mean;
    }
    else if (type == meanAndStandardDeviation)
    {
      const double mean = stat.getMean();
      const double stdDev = stat.getStandardDeviation();
      for (size_t i = 0; i < values.size(); ++i)
      {
        values[i] -= mean;
        if (stdDev > 10e-6)
          values[i] /= stdDev;
      }
    }

    VectorPtr res = objectVector(denseDoubleVectorClass(singletonEnumeration, doubleType), n);
    for (size_t i = 0; i < n; ++i)
    {
      DenseDoubleVectorPtr element = new DenseDoubleVector(singletonEnumeration, doubleType);
      element->setElement(0, Variable(values[i], doubleType));
      res->setElement(i, element);
    }
    
    return res;
  }  

protected:
  NormalizationType type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_PERCEPTION_H_
