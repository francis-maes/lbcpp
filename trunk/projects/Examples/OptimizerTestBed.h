/*
 *  OptimizerTestBed.h
 *  LBCpp
 *
 *  Created by Arnaud Schoofs on 9/05/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LBCPP_OPTIMIZER_TEST_BED_H_
# define LBCPP_OPTIMIZER_TEST_BED_H_

# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Core/Function.h>
# include <lbcpp/Function/ScalarVectorFunction.h>
# include <lbcpp/Optimizer/OptimizerContext.h>

namespace lbcpp
{
  
// TODO arnaud : move some static methods elsewhere
 
class UsefulFunctions
{
public:
  static DoubleMatrixPtr getLambdaMatrix(double alpha, size_t size)
  {
    DoubleMatrixPtr ret = new DoubleMatrix(size, size, 0.0);
    for (size_t i = 0; i < size; ++i) 
    {
      double value = pow(alpha, 0.5*i/(double)(size-1));  // WARNING : index i starts at 0
      ret->setValue(i, i, value);
    }
    return ret;
  }
};
  
class LinearTransformation 
{
public:
  static int sign(double x)
  {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
  }
  
  static void Tosz(const DenseDoubleVectorPtr& tab)
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
        c1 = 3.1;
      
      tab->setValue(i, sign(x)*exp(xhat + 0.049*(sin(c1*xhat) + sin(c2*xhat))));
    }
  }

  static void Tasy(const DenseDoubleVectorPtr& tab, double beta)
  {
    for (size_t i = 0; i < tab->getNumValues(); ++i) 
    {
      if (tab->getValue(i) > 0)
        tab->setValue(i, pow(tab->getValue(i), 1 + beta*i*sqrt(tab->getValue(i))/(tab->getNumValues()-1)));
    }
  }
};
  
  
  
  
// f(x) = ||z||^2 + f_opt, with z = x - x_opt
class SphereFunction : public ScalarVectorFunction
{
public:
  SphereFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  SphereFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;} // TODO arnaud
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues());
    
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    double norm = z->sumOfSquares();
    *output = norm + fopt;
  }
  
protected:
  friend class SphereFunctionClass;
  
  DenseDoubleVectorPtr xopt;
  double fopt;
};
  
// f(x) = \sum_{i=1}^{D} 10^(6*(i-1)/(D-1))*z_i^2 + f_opt
// with : D = dimension, z = Tosz(x - x_opt)
class EllipsoidalFunction : public ScalarVectorFunction
{
public:
  EllipsoidalFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  EllipsoidalFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;} // TODO arnaud
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())

    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    // z = Tosz(x - x_opt)
    LinearTransformation::Tosz(z);
    
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

class RastriginFunction : public ScalarVectorFunction
{
public:
  RastriginFunction(const DenseDoubleVectorPtr& xopt, double fopt) : xopt(xopt), fopt(fopt) {}
  RastriginFunction() {}
  
  virtual bool isDerivable() const
  {jassertfalse; return false;} // TODO arnaud
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    jassert(input->getNumValues() == xopt->getNumValues())
    
    // z = x - x_opt
    DenseDoubleVectorPtr z = input->cloneAndCast<DenseDoubleVector>();
    xopt->subtractFrom(z);
    
    // z = Tosz(x - x_opt)
    LinearTransformation::Tosz(z);
    
    // z = Tasy^0.2(Tosz(x - x_opt))
    LinearTransformation::Tasy(z, 0.2);
    
    DoubleMatrixPtr lambda = UsefulFunctions::getLambdaMatrix(10.0, z->getNumValues());
    
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
  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_TEST_BED_H_
