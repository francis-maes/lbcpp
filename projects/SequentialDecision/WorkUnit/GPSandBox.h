/*-----------------------------------------.---------------------------------.
| Filename: GPSandBox.h                    | Genetic Programming SandBox     |
| Author  : Francis Maes                   |                                 |
| Started : 22/05/2011 19:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_
# define LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include <lbcpp/Sampler/Sampler.h>

namespace lbcpp
{

////////////////////////////////////////////////////////

enum GPOperator
{
  gpAddition,
  gpSubtraction,
  gpMultiplication,
  gpDivision,
};

extern EnumerationPtr gpOperatorEnumeration;

enum GPPre
{
//  gpExp,
  gpLog,
  gpSquareRoot,
  gpInverse
};

extern EnumerationPtr gpPreEnumeration;
extern EnumerationPtr gpConstantEnumeration;

class GPExpr : public Object
{
public:
  virtual double compute(const std::vector<double>& x) const = 0;

  virtual size_t size() const = 0;
};

typedef ReferenceCountedObjectPtr<GPExpr> GPExprPtr;
extern ClassPtr gpExprClass;

extern ClassPtr variableGPExprClass;
extern ClassPtr unaryGPExprClass;
extern ClassPtr binaryGPExprClass;
extern ClassPtr constantGPExprClass;

class BinaryGPExpr;
typedef ReferenceCountedObjectPtr<BinaryGPExpr> BinaryGPExprPtr;

class UnaryGPExpr;
typedef ReferenceCountedObjectPtr<UnaryGPExpr> UnaryGPExprPtr;


class BinaryGPExpr : public GPExpr
{
public:
  BinaryGPExpr(GPExprPtr left, GPOperator op, GPExprPtr right)
    : left(left), op(op), right(right) {}
  BinaryGPExpr() {}

  virtual size_t size() const
    {return left->size() + 1 + right->size();}

  virtual double compute(const std::vector<double>& x) const
  {
    double l = left->compute(x);
    double r = right->compute(x);
    switch (op)
    {
    case gpAddition: return l + r;
    case gpSubtraction: return l - r;
    case gpMultiplication: return l * r;
    case gpDivision: return r ? l / r : 0.0;
    default: jassert(false); return 0.0;
    }
  }

  virtual String toShortString() const
  {
    const char* names[] = {"+", "-", "*", "/"};
    return T("(") + (left ? left->toShortString() : String("<null>")) + T(")") +
            names[op] +
           T("(") + (right ? right->toShortString() : String("<null>")) + T(")");
  }

  GPOperator getOperator() const
    {return op;}

  GPExprPtr getLeft() const
    {return left;}

  GPExprPtr getRight() const
    {return right;}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const BinaryGPExprPtr& other = otherObject.staticCast<BinaryGPExpr>();
    if (op != other->op)
      return (int)op - (int)other->op;
    else
    {
      int res = Variable(left).compare(other->left);
      if (res != 0)
        return res;
      else
        return Variable(right).compare(other->right);
    }
  }
  virtual void clone(ExecutionContext& context, const ObjectPtr& other) const
  {
    GPExpr::clone(context, other);
    const BinaryGPExprPtr& o = other.staticCast<BinaryGPExpr>();
    o->left = left->cloneAndCast<GPExpr>(context);
    o->right = right->cloneAndCast<GPExpr>(context);
  }
 

protected:
  friend class BinaryGPExprClass;

  GPExprPtr left;
  GPOperator op;
  GPExprPtr right;
};

class UnaryGPExpr : public GPExpr
{
public:
  virtual size_t size() const
    {return 1 + expr->size();}

  virtual double compute(const std::vector<double>& x) const
  {
    double e = expr->compute(x);
    switch (pre)
    {
    //case gpSin: return sin(e);
    //case gpCos: return cos(e);
    //case gpExp: return exp(e);
    case gpLog: return e <= 0.0 ? 0.0 : log(e);
    case gpSquareRoot: return sqrt(e);
    //case gpSquare: return e * e;
    case gpInverse: return e != 0.0 ? 1.0 / e : 0.0;
    default: jassert(false); return 0.0;
    };
  }

  virtual String toShortString() const
    {return Variable(pre, gpPreEnumeration).toShortString() + T("(") + (expr ? expr->toShortString() : String("<null>")) + T(")");}

  GPPre getOperator() const
    {return pre;}

  GPExprPtr getExpression() const
    {return expr;}

  virtual int compare(const ObjectPtr& otherObject) const
  {
    const UnaryGPExprPtr& other = otherObject.staticCast<UnaryGPExpr>();
    if (pre != other->pre)
      return (int)pre - (int)other->pre;
    else
      return Variable(expr).compare(other->expr);
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& other) const
  {
    GPExpr::clone(context, other);
    other.staticCast<UnaryGPExpr>()->expr = expr->cloneAndCast<GPExpr>(context);
  }
 
protected:
  friend class UnaryGPExprClass;

  GPPre pre;
  GPExprPtr expr;
};

class VariableGPExpr : public GPExpr
{
public:
  VariableGPExpr(size_t index = 0)
    : index(index) {}

  virtual size_t size() const
    {return 1;}

  virtual double compute(const std::vector<double>& x) const
    {jassert(index.getInteger() < (int)x.size()); return x[index.getInteger()];}

  virtual String toShortString() const
    {return index.toShortString();}

  size_t getIndex() const
    {return (size_t)index.getInteger();}

  virtual int compare(const ObjectPtr& otherObject) const
    {return index.compare(otherObject.staticCast<VariableGPExpr>()->index);}

protected:
  friend class VariableGPExprClass;

  Variable index;
};

typedef ReferenceCountedObjectPtr<VariableGPExpr> VariableGPExprPtr;

class ConstantGPExpr : public GPExpr
{
public:
  ConstantGPExpr(double value = 0.0)
    : value(value) {}

  virtual size_t size() const
    {return 1;}

  virtual double compute(const std::vector<double>& x) const
    {return value;}

  virtual String toShortString() const
    {return String(getValue());}

  double getValue() const
    {return value;}
 
  void setValue(double value)
    {this->value = value;}

  virtual int compare(const ObjectPtr& otherObject) const
    {return Variable(value).compare(otherObject.staticCast<ConstantGPExpr>()->value);}
 
protected:
  friend class ConstantGPExprClass;

  double value;
};

typedef ReferenceCountedObjectPtr<ConstantGPExpr> ConstantGPExprPtr;

////////////////////////////////////////////////////////

extern EnumerationPtr gpExprLabelsEnumeration;

class GPExprSamplingContext;
typedef ReferenceCountedObjectPtr<GPExprSamplingContext> GPExprSamplingContextPtr;

extern ClassPtr gpExprSamplingContextClass;


class GPExprSamplingContext : public Object
{
public:
  GPExprSamplingContext(GPExprSamplingContextPtr parent, size_t parentLabel)
    : parent(parent), depth(parent->depth + 1), parentLabel(parentLabel), grandParentLabel(parent->parentLabel){}
  GPExprSamplingContext() : parentLabel(gpExprLabelsEnumeration->getNumElements()),
    grandParentLabel(gpExprLabelsEnumeration->getNumElements()), depth(0) {}

private:
  friend class GPExprSamplingContextClass;

  GPExprSamplingContextPtr parent;
  size_t parentLabel;
  size_t grandParentLabel;
  size_t depth;
};

// GPExprSamplingContext -> DoubleVector
class GPExprSamplingContextFeatureGenerator : public CompositeFunction
{
public:
  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t context = builder.addInput(gpExprSamplingContextClass, T("context"));
    size_t depth = builder.addFunction(getVariableFunction(T("depth")), context);
    size_t parentLabel = builder.addFunction(getVariableFunction(T("parentLabel")), context);
    size_t grandParentLabel = builder.addFunction(getVariableFunction(T("grandParentLabel")), context);

    builder.startSelection();
      builder.addFunction(defaultPositiveDoubleFeatureGenerator(10, 0, 1), depth);
      builder.addFunction(enumerationFeatureGenerator(), parentLabel);
      builder.addFunction(enumerationFeatureGenerator(), grandParentLabel);
    builder.finishSelectionWithFunction(concatenateFeatureGenerator());
  }
};

class GPExprSampler : public CompositeSampler
{
public:
  GPExprSampler(DiscreteSamplerPtr classSampler, EnumerationPtr variablesEnumeration, size_t depth)
    : classSampler(classSampler), variablesEnumeration(variablesEnumeration), depth(depth)
  {
    featureGenerator = new GPExprSamplingContextFeatureGenerator();
    featureGenerator->initialize(defaultExecutionContext(), gpExprSamplingContextClass); 
  }

  GPExprSampler() {}

  virtual String toShortString() const
    {return classSampler->toShortString();}

  SamplerPtr createSubExprSampler() const
  {
    if (depth < 5)
      return new GPExprSampler(classSampler, variablesEnumeration, depth + 1);
    else
      return constantSampler(new VariableGPExpr());
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    if (samplers.empty())
    {
      std::vector<SamplerPtr>& samplers = const_cast<GPExprSampler* >(this)->samplers;
      // binary
      for (size_t i = 0; i < gpOperatorEnumeration->getNumElements(); ++i)
        samplers.push_back(objectCompositeSampler(binaryGPExprClass,
           createSubExprSampler(), constantSampler(Variable(i, gpOperatorEnumeration)), createSubExprSampler()));

      // unary
      for (size_t i = 0; i < gpPreEnumeration->getNumElements(); ++i)
        samplers.push_back(objectCompositeSampler(unaryGPExprClass,
           constantSampler(Variable(i, gpPreEnumeration)), createSubExprSampler()));

      // constant
      samplers.push_back(constantSampler(new ConstantGPExpr(0.0)));

      // variables
      for (size_t i = 0; i < 4; ++i)
        samplers.push_back(objectCompositeSampler(variableGPExprClass, constantSampler(Variable(i, variablesEnumeration))));

      samplers.push_back(classSampler);
    }

    GPExprSamplingContextPtr samplingContext;
    if (inputs)
      samplingContext = inputs[0].getObjectAndCast<GPExprSamplingContext>();
    else
      samplingContext = new GPExprSamplingContext();

    Variable features = featureGenerator->compute(context, samplingContext);

    size_t index = (size_t)classSampler->sample(context, random, &features).getInteger();
    jassert(index < samplers.size() - 1);

    Variable subSamplingContext = new GPExprSamplingContext(samplingContext, index);
    return samplers[index]->sample(context, random, &subSamplingContext);
  }

  virtual void makeSubExamples(const ContainerPtr& inputs, const ContainerPtr& samples, const DenseDoubleVectorPtr& weights, std::vector<ContainerPtr>& subInputs, std::vector<ContainerPtr>& subSamples, std::vector<ContainerPtr>& subWeights) const
    {jassert(false);}
 
  size_t matchExpression(const GPExprPtr& expression) const
  {
    TypePtr type = expression->getClass();
    if (type == binaryGPExprClass)
    {
      const BinaryGPExprPtr& expr = expression.staticCast<BinaryGPExpr>();
      GPOperator op = expr->getOperator();
      return op - gpAddition;
    }
    else if (type == unaryGPExprClass)
    {
      const UnaryGPExprPtr& expr = expression.staticCast<UnaryGPExpr>();
      GPPre pre = expr->getOperator();
      return 4 + (pre - gpLog);
    }
    else if (type == constantGPExprClass)
    {
      //const ConstantGPExprPtr& expr = expression.staticCast<ConstantGPExpr>();
      return 7;// + expr->getConstantIndex();
    }
    else if (type == variableGPExprClass)
    {
      const VariableGPExprPtr& expr = expression.staticCast<VariableGPExpr>();
      return 8 + expr->getIndex();
    }
    else
    {
      jassert(false);
      return false;
    }
  }

  void createClassificationExamplesRecursively(ExecutionContext& context, const GPExprSamplingContextPtr& samplingContext, const GPExprPtr& expression, const ObjectVectorPtr& features, const VectorPtr& labels)
  {
    size_t index = matchExpression(expression);
    features->append(featureGenerator->compute(context, samplingContext));
    labels->append(Variable(index, gpExprLabelsEnumeration));

    if (expression.dynamicCast<UnaryGPExpr>())
    {
      GPExprSamplingContextPtr subSamplingContext = new GPExprSamplingContext(samplingContext, index);
      createClassificationExamplesRecursively(context, subSamplingContext, expression.staticCast<UnaryGPExpr>()->getExpression(), features, labels);
      return;
    }
    if (expression.dynamicCast<BinaryGPExpr>())
    {
      GPExprSamplingContextPtr subSamplingContext = new GPExprSamplingContext(samplingContext, index);
      const BinaryGPExprPtr& binaryExpression = expression.staticCast<BinaryGPExpr>();
      createClassificationExamplesRecursively(context, subSamplingContext, binaryExpression->getLeft(), features, labels);
      createClassificationExamplesRecursively(context, subSamplingContext, binaryExpression->getRight(), features, labels);
      return;
    }
  }
  
  void makeClassificationData(ExecutionContext& context, const ContainerPtr& samples, ContainerPtr& inputs, ContainerPtr& outputs)
  {
    size_t n = samples->getNumElements();

    ObjectVectorPtr featureVectors = new ObjectVector(doubleVectorClass(), 0);
    VectorPtr labels = vector(gpExprLabelsEnumeration, 0);
    
    for (size_t i = 0; i < n; ++i)
    {
      GPExprPtr expression = samples->getElement(i).getObjectAndCast<GPExpr>();
      createClassificationExamplesRecursively(context, new GPExprSamplingContext(), expression, featureVectors, labels);
    }

    inputs = featureVectors;
    outputs = labels;
  }

  virtual void learn(ExecutionContext& context, const ContainerPtr& trainingInputs, const ContainerPtr& trainingSamples, const DenseDoubleVectorPtr& trainingWeights,
                                                const ContainerPtr& validationInputs, const ContainerPtr& validationSamples, const DenseDoubleVectorPtr& supervisionWeights)
  {
    ContainerPtr subInputs, subOutputs;
    makeClassificationData(context, trainingSamples, subInputs, subOutputs);
    classSampler->learn(context, subInputs, subOutputs);
  }

protected:
  friend class GPExprSamplerClass;

  DiscreteSamplerPtr classSampler;
  FunctionPtr featureGenerator;
  EnumerationPtr variablesEnumeration;
  size_t depth;
};

////////////////////////////////////////////////////////

// GPExpr -> Double
class GPObjectiveFunction : public SimpleUnaryFunction
{
public:
  GPObjectiveFunction(const std::vector<std::pair< std::vector<double> , double> >& examples, double lambda)
    : SimpleUnaryFunction(gpExprClass, doubleType), examples(examples), lambda(lambda) {}
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExprPtr& expression = input.getObjectAndCast<GPExpr>();

    double res = 0.0;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      double prediction = expression->compute(examples[i].first);
      res += fabs(prediction - examples[i].second);
    }
    return res / (double)examples.size() + lambda * expression->size();
  }
 
protected:
  double lambda;
  std::vector<std::pair<std::vector<double> , double> > examples;
};

// GPExpr -> Double
class GPStructureObjectiveFunction : public SimpleUnaryFunction
{
public:
  GPStructureObjectiveFunction(FunctionPtr objectiveFunction)
    : SimpleUnaryFunction(gpExprClass, doubleType), objectiveFunction(objectiveFunction)
  {
    objectiveFunction->initialize(defaultExecutionContext(), gpExprClass);
  }

  size_t getNumConstants(const GPExprPtr& expression) const
  {
    if (expression.dynamicCast<ConstantGPExpr>())
      return 1;
    else if (expression.dynamicCast<BinaryGPExpr>())
    {
      const BinaryGPExprPtr& expr = expression.staticCast<BinaryGPExpr>();
      return getNumConstants(expr->getLeft()) + getNumConstants(expr->getRight());
    }
    else if (expression.dynamicCast<UnaryGPExpr>())
    {
      const UnaryGPExprPtr& expr = expression.staticCast<UnaryGPExpr>();
      return getNumConstants(expr->getExpression());
    }
    return 0;
  }


  // DenseDoubleVector -> Double
  struct Objective : public SimpleUnaryFunction
  {
    Objective(FunctionPtr finalObjective, const GPExprPtr& structure)
      : SimpleUnaryFunction(denseDoubleVectorClass(), doubleType),
        finalObjective(finalObjective), structure(structure)
    {
    }

    void setConstantsRecursively(const GPExprPtr& expression, const DenseDoubleVectorPtr& constants, size_t& index) const
    {
      if (expression.dynamicCast<ConstantGPExpr>())
      {
        expression.staticCast<ConstantGPExpr>()->setValue(constants->getValue(index));
        ++index;
      }
      else if (expression.dynamicCast<BinaryGPExpr>())
      {
        const BinaryGPExprPtr& expr = expression.staticCast<BinaryGPExpr>();
        setConstantsRecursively(expr->getLeft(), constants, index);
        setConstantsRecursively(expr->getRight(), constants, index);
      }
      else if (expression.dynamicCast<UnaryGPExpr>())
      {
        const UnaryGPExprPtr& expr = expression.staticCast<UnaryGPExpr>();
        setConstantsRecursively(expr->getExpression(), constants, index);
      }
    }

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      DenseDoubleVectorPtr constants = input.getObjectAndCast<DenseDoubleVector>();
      GPExprPtr expression = structure->cloneAndCast<GPExpr>();
      size_t index = 0;
      setConstantsRecursively(expression, constants, index);
      Variable res = finalObjective->compute(context, expression);
      return res;
    }

  protected:
    FunctionPtr finalObjective;
    GPExprPtr structure;
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExprPtr& expression = input.getObjectAndCast<GPExpr>();

    size_t numConstants = getNumConstants(expression);
    if (numConstants == 0)
      return objectiveFunction->compute(context, input);

    context.enterScope(T("Optimizing constants in ") + expression->toShortString());
    // Enumeration des constantes
    DefaultEnumerationPtr constantsEnumeration = new DefaultEnumeration(T("constants"));
    for (size_t i = 0; i < numConstants; ++i)
      constantsEnumeration->addElement(context, T("C") + String((int)i));

    // Sampler de DenseDoubleVector pour les constantes
    SamplerPtr constantsSampler = independentDoubleVectorSampler((EnumerationPtr)constantsEnumeration, gaussianSampler());

    // opmitizer context and state
    FunctionPtr objective = new Objective(objectiveFunction, expression);
    objective->initialize(context, denseDoubleVectorClass(constantsEnumeration));

    OptimizerContextPtr optimizerContext = synchroneousOptimizerContext(context, objective);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(constantsSampler);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(3, 100, 10, 0.0, true);
    optimizer->compute(context, optimizerContext, optimizerState);
    
    Variable res = optimizerState->getBestScore();
    context.leaveScope(res);
    return res;
  }

protected:
  FunctionPtr objectiveFunction;
};

////////////////////////////////////////////////////////

class GPSandBox : public WorkUnit
{
public:
  GPSandBox() : lambda(0.01)
  {
    numIterations = 100;
    populationSize = 1000;
    numBests = 200;
  }

  virtual Variable run(ExecutionContext& context)
  {
    RandomGeneratorPtr random = new RandomGenerator();

    std::vector<std::pair<std::vector<double> , double> > examples;
    for (size_t i = 0; i < 1000; ++i)
    {
      std::vector<double> x(4);

      x[0] = random->sampleDouble(0.0, 6.0);
      x[1] = random->sampleDouble(0.0, 6.0);
      x[2] = random->sampleDouble(0.0, 6.0);
      x[3] = random->sampleDouble(0.0, 6.0);
      double y = x[0] + 8.6 * x[1];// + x[2] + x[3];// 10.0 / (5.0 + (x[0] - 2) * (x[0] - 2) + (x[1] - 2) * (x[1] - 2));
//      double y = 1.0 + x[0] + x[1];//cos(x[0] * (1 + cos(x[0])));
      examples.push_back(std::make_pair(x, y));
    }

    FunctionPtr objective = new GPStructureObjectiveFunction(new GPObjectiveFunction(examples, lambda));
    if (!objective->initialize(context, gpExprClass))
      return false;

    DefaultEnumerationPtr variablesEnumeration = new DefaultEnumeration(T("variables"));
    variablesEnumeration->addElement(context, T("x1"));
    variablesEnumeration->addElement(context, T("x2"));
    variablesEnumeration->addElement(context, T("x3"));
    variablesEnumeration->addElement(context, T("x4"));
    SamplerPtr sampler = createExpressionSampler(variablesEnumeration);

    //for (size_t i = 0; i < 10; ++i)
    //  context.informationCallback(sampler->sample(context, random).toShortString());

    return optimize(context, sampler, objective);
  }

  SamplerPtr createExpressionSampler(EnumerationPtr variablesEnumeration) const
    {return new GPExprSampler(enumerationSampler(gpExprLabelsEnumeration), variablesEnumeration, 1);}

  bool optimize(ExecutionContext& context, const SamplerPtr& sampler, const FunctionPtr& objective)
  {
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objective);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, 0.0, true, false);
    optimizer->compute(context, optimizerContext, optimizerState);

    return true;
  }

protected:
  friend class GPSandBoxClass;

  double lambda;
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_
