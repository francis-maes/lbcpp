/*-----------------------------------------.---------------------------------.
| Filename: OptimizerTestBed               | Contains functions useful to    |
| Author  : Arnaud Schoofs                 | asses the quality of an         |
| Started : 09/05/2011                     | Optimizer                       |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*
 ** SOURCE :  http://coco.lri.fr/downloads/download10.61/bbobdocfunctionsdef.pdf
 */


#ifndef LBCPP_OPTIMIZER_TEST_BED_H_
# define LBCPP_OPTIMIZER_TEST_BED_H_

# include <lbcpp/Function/ScalarVectorFunction.h>
# include <algorithm>

// TODO arnaud : commenter
// TODO arnaud : clean code
// TODO arnaud : utiliser sumofsquare

namespace lbcpp
{ 

/**
 * This namespace contains some usefull functions to define the
 * "Real-Parameter Black-Box Optimization Benchmarking Noiseless Functions"
 */
namespace testbed 
{
  /**
   * @return -1 if x < 0, 0 if x = 0 and 1 otherwise
   */
  int sign(double x)
  {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
  }
  
  /**
   * Returns a diagonal matrix with i-th diagonal element as \lambda_{ii} = \alpha^{\frac{i-1}{2*(D-1)}}
   * @param alpha \alpha
   * @param D dimension
   */
  DoubleMatrixPtr getLambdaMatrix(double alpha, size_t dimension)
  {
    DoubleMatrixPtr ret = new DoubleMatrix(dimension, dimension, 0.0);
    for (size_t i = 1; i <= dimension; ++i) 
    {
      double value = pow(alpha, 0.5*((double)(i-1)/(double)(dimension-1)));
      ret->setValue((i-1), (i-1), value);
    }
    return ret;
  }
  
  /**
   * Tosz function used to introduce non-linearity and assimetry.
   * @param tab vector to which the transformation is applied.
   */
  void transformTosz(const DenseDoubleVectorPtr& tab)
  {
    for (size_t i = 0; i < tab->getNumValues(); i++)
    {
      double x = tab->getValue(i);
      double xhat;
      if (x != 0)
        xhat = log(fabs(x));
      else
        xhat = 0.0;
      
      double c1;
      if (x > 0)
        c1 = 10.0;
      else
        c1 = 5.5;
      
      double c2;
      if (x > 0)
        c2 = 7.9;
      else
        c2 = 3.1;
      
      tab->setValue(i, sign(x)*exp(xhat + 0.049*(sin(c1*xhat) + sin(c2*xhat))));
    }
  }
  
  /**
   * Tasy function used to introduce non-linearity and assimetry.
   * @param beta \beta
   * @param tab vector to which the transformation is applied.
   */
  void transformTasy(const DenseDoubleVectorPtr& tab, double beta)
  {
    for (size_t i = 0; i < tab->getNumValues(); ++i) 
    {
      if (tab->getValue(i) > 0)
        tab->setValue(i, pow(tab->getValue(i), 1 + beta*i*sqrt(tab->getValue(i))/(double)(tab->getNumValues()-1)));
    }
  }
  
  /**
   * Generates a random orthogonal (rotation) matrix.
   * The orthogonal matrix is generated from standard normally distributed entries by Gramn-Schmidt orthonormalization.
   * (source: http://coco.gforge.inria.fr/doku.php?id=downloads )
   * @param DIM dimension of the matrix to return.
   * @return random orthogonal matrix.
   */ 
  DoubleMatrixPtr getRotationMatrix(int DIM)
  {
    // TODO : maybe use a static variable for a dedicated RandomGenerator initialized with a constant seed (so that two executions of a program give the same results).
    DoubleMatrixPtr R = independentDoubleMatrixSampler(DIM, DIM, gaussianSampler())->sample(defaultExecutionContext(), defaultExecutionContext().getRandomGenerator()).getObjectAndCast<DoubleMatrix>();
    
    double prod;
    int i, j, k;
    
    for (i = 0; i < DIM; i++)
    {
      for (j = 0; j < i; j++)
      {
        prod = 0;
        for (k = 0; k < DIM; k++)
          prod += R->getValue(k,i) * R->getValue(k,j);
        for (k = 0; k < DIM; k++)
          R->setValue(k,i, R->getValue(k,i)-prod*R->getValue(k,j));
      }
      prod = 0;
      for (k = 0; k < DIM; k++)
        prod += R->getValue(k,i) * R->getValue(k,i);
      for (k = 0; k < DIM; k++)
        R->setValue(k,i, R->getValue(k,i)/sqrt(prod));
    }
    return R;
  }
  
  /**
   * Penalty boundary handling term.
   * @param vector vector used to caculate the penalty boundary handling term.
   * @return f_{pen}
   */
  double fpen(const DenseDoubleVectorPtr& vector)
  {
    double result = 0.0;
    for (size_t i = 0; i < vector->getNumValues(); i++) 
    {
      double value = std::max(0.0, fabs(vector->getValue(i)) - 5);
      result += value*value;
    }
    return result;
  }
}; 
  
  
class SphereFunction : public ScalarVectorFunction
{
public:
  SphereFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  SphereFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues());
    
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();  // z = x
    xopt->subtractFrom(z);  // z = x - x_opt
    
    double squarenorm = z->sumOfSquares();
    *output = squarenorm + fopt;
  }
  
protected:
  friend class SphereFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};
  
// f2(x) = \sum_{i=1}^{D} 10^(6*(i-1)/(D-1))*z_i^2 + f_opt
// with : D = dimension, z = Tosz(x - x_opt)
class EllipsoidalFunction : public ScalarVectorFunction
{
public:
  EllipsoidalFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  EllipsoidalFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())

    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    // z = Tosz(x - x_opt)
    testbed::transformTosz(z);
    
    double result = 0;
    for (size_t i = 0; i < z->getNumValues(); ++i)    // WARNING: index i starts at 0
      result += pow(10.0, 6.0*((double)i/(double)(z->getNumValues()-1)))*z->getValue(i)*z->getValue(i);

    *output = result + fopt;
  }
  
protected:
  friend class EllipsoidalFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};

// f3
class RastriginFunction : public ScalarVectorFunction
{
public:
  RastriginFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  RastriginFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    // z = Tosz(x - x_opt)
    testbed::transformTosz(z);
    
    // z = Tasy^0.2(Tosz(x - x_opt))
    testbed::transformTasy(z, 0.2);
    
    DoubleMatrixPtr lambda = testbed::getLambdaMatrix(10.0, z->getNumValues());
    
    // z = lambda * Tasy^0.2(Tosz(x - x_opt))
    for (size_t i = 0; i < z->getNumValues(); ++i) 
    {
      z->setValue(i, lambda->getValue(i,i)*z->getValue(i));
    }
    
    double term = z->getNumValues();
    for (size_t i = 0; i < z->getNumValues(); ++i)
      term -= cos(2*M_PI*z->getValue(i));
    term *= 10;
    
    *output = term + z->sumOfSquares() + fopt;
  
  }
  
protected:
  friend class RastriginFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};

// f4
// don't converge easily
  /*
  size_t numIterations = 30;
  size_t populationSize = 1000;
  size_t numBests = 10;
  double slowingFactor = 0.3;
   */
  
class BucheRastriginFunction : public ScalarVectorFunction
{
public:
  BucheRastriginFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  BucheRastriginFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
   
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    testbed::transformTosz(z);
    
    for (size_t i = 0; i < z->getNumValues(); i++) 
    {
      if (z->getValue(i) > 0 && i % 2 == 0) // WARNING i starts at 0
        z->setValue(i, z->getValue(i) * 10 * pow(10.0, 0.5*i/(double)(z->getNumValues()-1)));
      else
        z->setValue(i, z->getValue(i) * pow(10.0, 0.5*i/(double)(z->getNumValues()-1)));
    }
    
    double term = z->getNumValues();
    for (size_t i = 0; i < z->getNumValues(); ++i)
      term -= cos(2*M_PI*z->getValue(i));
    term *= 10;
    
    *output = term + z->sumOfSquares() + 100*testbed::fpen(input) + fopt;
  }
  
protected:
  friend class BucheRastriginFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};

// f5
// TODO arnaud : don't converge, maybe bugged
class LinearSlopeFunction : public ScalarVectorFunction 
{
public:
  LinearSlopeFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  LinearSlopeFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    
    DenseDoubleVectorPtr s = new DenseDoubleVector(input->getNumValues(), 0.0);
    for (size_t i = 0; i < input->getNumValues(); i++)
      s->setValue(i, testbed::sign(xopt->getValue(i))*pow(10.0, (double)i/(double)(input->getNumValues()-1)));  // WARNING i starts at 0
    
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    for (size_t i = 0; i < z->getNumValues(); i++)
      if (xopt->getValue(i)*input->getValue(i) > 25) 
        z->setValue(i, xopt->getValue(i));

    double sum = 0.0;
    for (size_t i = 0; i < z->getNumValues(); i++)
    {
      sum += 5*fabs(s->getValue(i)) - s->getValue(i)*z->getValue(i);
    }
     *output = sum + fopt;
  }
  
protected:
  friend class LinearSlopeFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};
  
// f6 
class AttractiveSectorFunction : public ScalarVectorFunction
{
public:
  AttractiveSectorFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  AttractiveSectorFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    DoubleMatrixPtr Q = testbed::getRotationMatrix(z->getNumValues());
    DoubleMatrixPtr lambda = testbed::getLambdaMatrix(10.0, z->getNumValues());
    DoubleMatrixPtr R = testbed::getRotationMatrix(z->getNumValues());
    DoubleMatrixPtr Qlambda = Q->multiplyBy(lambda);
    DoubleMatrixPtr QlambdaR = Qlambda->multiplyBy(R);
    DenseDoubleVectorPtr newZ =  QlambdaR->multiplyVector(z);

    double sum = 0.0;
    for (size_t i = 0; i < z->getNumValues(); i++) 
    {
      double si = 1;
      if (newZ->getValue(i) * xopt->getValue(i) > 0) {
        si = 100;
      }
      double product = si*newZ->getValue(i);
      sum += product*product;
    }
    DenseDoubleVectorPtr ssum = new DenseDoubleVector(1, sum);
    testbed::transformTosz(ssum);
    
    double power = pow(ssum->getValue(0), 0.9);
    *output = power + fopt;

  }
  
protected:
  friend class AttractiveSectorFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};

// f7  
class StepEllipsoidalFunction : public ScalarVectorFunction
{
public:
  StepEllipsoidalFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  StepEllipsoidalFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    DoubleMatrixPtr lambda = testbed::getLambdaMatrix(10.0, z->getNumValues());
    DoubleMatrixPtr R = testbed::getRotationMatrix(z->getNumValues());
    DoubleMatrixPtr Rlambda = lambda->multiplyBy(R);
    DenseDoubleVectorPtr zhat = Rlambda->multiplyVector(z);
    
    DenseDoubleVectorPtr ztilde = new DenseDoubleVector(z->getNumValues(), 0.0);
    for (size_t i = 0; i < z->getNumValues(); i++)
    {
      if (zhat->getValue(i) > 0.5) 
        ztilde->setValue(i, floor(0.5+zhat->getValue(i))); 
      else
        ztilde->setValue(i, floor(0.5+10*zhat->getValue(i))/(double)10);
    }
    
    z = testbed::getRotationMatrix(z->getNumValues())->multiplyVector(ztilde);
    
    double sum = 0.0;
    for (size_t i = 0; i < z->getNumValues(); i++) 
      sum += pow(10.0, (double)(2*i)/(double)(z->getNumValues()-1))*z->getValue(i)*z->getValue(i);
    *output = 0.1*std::max(fabs(zhat->getValue(0))/(double)10000, sum) + testbed::fpen(input) + fopt;
  }
  
protected:
  friend class StepEllipsoidalFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};

// f8
class RosenbrockFunction : public ScalarVectorFunction
{
public:
  RosenbrockFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  RosenbrockFunction() {}
  
  virtual bool isDerivable() const
  {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    z->multiplyByScalar(std::max(1.0,sqrt((double)z->getNumValues())/(double)8));
    DenseDoubleVectorPtr ones = new DenseDoubleVector(z->getNumValues(), 1);
    ones->addTo(z);
    
    double sum = 0.0;
    for (size_t i = 0; i < z->getNumValues() - 1; i++) 
    {
      double zi = z->getValue(i);
      double zisquare = zi*zi;
      double diff1 = zisquare - z->getValue(i+1);
      double diff2 = zi - 1;
      sum += 100*diff1*diff1 + diff2*diff2;
    }
    *output = sum + fopt;
    
  }
  
protected:
  friend class RosenbrockFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
  
};
  
// f9
// TODO arnaud : don't converge, maybe bugged
class RosenbrockRotatedFunction : public ScalarVectorFunction
{
public:
  RosenbrockRotatedFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  RosenbrockRotatedFunction() {}
  
  virtual bool isDerivable() const
  {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    // z = x - x_opt
    DenseDoubleVectorPtr z = new DenseDoubleVector(input->getNumValues(), 0.0);
    z = testbed::getRotationMatrix(z->getNumValues())->multiplyVector(input);
    
    z->multiplyByScalar(std::max(1.0,sqrt((double)z->getNumValues())/(double)8));
    DenseDoubleVectorPtr half = new DenseDoubleVector(z->getNumValues(), 0.5);
    half->addTo(z);
    
    double sum = 0.0;
    for (size_t i = 0; i < z->getNumValues() - 1; i++) 
    {
      double zi = z->getValue(i);
      double zisquare = zi*zi;
      double diff1 = zisquare - z->getValue(i+1);
      double diff2 = zi - 1;
      sum += 100*diff1*diff1 + diff2*diff2;
    }
    *output = sum + fopt;
    
  }
  
protected:
  friend class RosenbrockRotatedFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
  
};

// f10  
class IllEllipsoidalFunction : public ScalarVectorFunction
{
public:
  IllEllipsoidalFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  IllEllipsoidalFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues());
    
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    DenseDoubleVectorPtr newZ = testbed::getRotationMatrix(z->getNumValues())->multiplyVector(z);
    
    z = newZ;    
    testbed::transformTosz(z);
    
    double result = 0;
    for (size_t i = 0; i < z->getNumValues(); ++i)    // WARNING: index i starts at 0
      result += pow(10.0, 6.0*((double)i/(double)(z->getNumValues()-1)))*z->getValue(i)*z->getValue(i);
    
    *output = result + fopt;
    
  }
  
protected:
  friend class IllEllipsoidalFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};
 
// f11  
class DiscusFunction : public ScalarVectorFunction
{
public:
  DiscusFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  DiscusFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues());
    
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    DenseDoubleVectorPtr newZ = testbed::getRotationMatrix(z->getNumValues())->multiplyVector(z);
    
    z = newZ;    
    testbed::transformTosz(z);
    
    double sum = 1000000*z->getValue(0)*z->getValue(0);
    for (size_t i = 1; i < z->getNumValues(); i++) 
    {
      double value = z->getValue(i);
      sum += value*value;
    }
    
    *output = sum + fopt;
  }
  
protected:
  friend class DiscusFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};

// f12  
class BentCigarFunction : public ScalarVectorFunction
{
public:
  BentCigarFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  BentCigarFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues());
    
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    DoubleMatrixPtr R = testbed::getRotationMatrix(z->getNumValues());
    DenseDoubleVectorPtr newZ = R->multiplyVector(z);
    testbed::transformTasy(newZ, 0.5);
    z = R->multiplyVector(newZ);
    
    double sum = 0.0;
    for (size_t i = 1; i < z->getNumValues(); i++) 
    {
      double value = z->getValue(i);
      sum += value*value;
    }
    
    *output = z->getValue(0)*z->getValue(0) + 1000000*sum + fopt;
  }
  
protected:
  friend class BentCigarFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};
  
// f13  
class SharpRidgeFunction : public ScalarVectorFunction
{
public:
  SharpRidgeFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  SharpRidgeFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    DoubleMatrixPtr Q = testbed::getRotationMatrix(z->getNumValues());
    DoubleMatrixPtr lambda = testbed::getLambdaMatrix(10.0, z->getNumValues());
    DoubleMatrixPtr R = testbed::getRotationMatrix(z->getNumValues());
    DoubleMatrixPtr Qlambda = Q->multiplyBy(lambda);
    DoubleMatrixPtr QlambdaR = Qlambda->multiplyBy(R);
    DenseDoubleVectorPtr newZ = QlambdaR->multiplyVector(z);
    
    z = newZ;
    
    double sum = 0.0;
    for (size_t i = 1; i < z->getNumValues(); i++) 
    {
      double value = z->getValue(i);
      sum += value*value;
    }
    
    *output = z->getValue(0)*z->getValue(0) + 100*sqrt(sum) + fopt;
    
  }
  
protected:
  friend class SharpRidgeFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
  
};

// f14  
class DifferentPowersFunction : public ScalarVectorFunction
{
public:
  DifferentPowersFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  DifferentPowersFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;}  // not implemented
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    DenseDoubleVectorPtr newZ = testbed::getRotationMatrix(z->getNumValues())->multiplyVector(z);
    z = newZ;
    
    double sum = 0.0;
    for (size_t i = 0; i < z->getNumValues(); i++) 
      sum += pow(fabs(z->getValue(i)), 2 + (double)(4*i)/(double)(z->getNumValues()-1));
    
    *output = sqrt(sum) + fopt;
  }
  
protected:
  friend class DifferentPowersFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_TEST_BED_H_
