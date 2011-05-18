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

class LinearTransformation 
{
public:
  static int sign(double x)
  {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
  }
  
  static void Tosz(std::vector<double>& inputTab) 
  {
    for (size_t i = 0; i < inputTab.size(); i++)
    {
      double x = inputTab[i];
      double xhat;
      if (x != 0)
        xhat = log(fabs(x));
      else
        xhat = 0;
      
      double c1;
      if (x > 0)
        c1 = 10;
      else
        c1 = 5.5;
      
      double c2;
      if (x > 0)
        c2 = 7.9;
      else
        c1 = 3.1;
      
      inputTab[i] = sign(x)*exp(xhat + 0.049*(sin(c1*xhat) + sin(c2*xhat)));
    }
  }
};
  
// f(x) = ||z||^2 + f_opt
class SphereFunction : public ScalarVectorFunction
{
public:
  SphereFunction(const std::vector<double>& coefs, double fopt) : coefs(coefs), fopt(fopt) {}
  SphereFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;} // TODO arnaud
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    const std::vector<double>& inputTab = input->getValues();
    
    jassert(coefs.size() == inputTab.size());
    double result = 0;
    for (size_t i = 0; i < coefs.size(); i++)
    {
      double diff = inputTab[i] - coefs[i];
      result += diff*diff;
    }
    result += fopt;
    *output = result;
  }
  
protected:
  friend class SphereFunctionClass;
  
  std::vector<double> coefs;
  double fopt;
};
  
class EllipsoidalFunction : public ScalarVectorFunction
{
public:
  EllipsoidalFunction(const std::vector<double>& coefs, double fopt) : coefs(coefs), fopt(fopt) {}
  EllipsoidalFunction() {}
  
  virtual bool isDerivable() const
    {jassertfalse; return false;} // TODO arnaud
  
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const
  {
    const std::vector<double>& inputTab = input->getValues();
    jassert(coefs.size() == inputTab.size());
    
    std::vector<double> diff;
    diff.resize(inputTab.size());
    for (size_t i = 0; i < coefs.size(); i++)
      diff[i] = coefs[i] - inputTab[i];
    LinearTransformation::Tosz(diff);
    
    double result = 0;
    for (size_t i = 1; i <= coefs.size(); i++)
      result += pow(10.0, 6.0*((i-1)/(coefs.size()-1)))*diff[i]*diff[i];

    result += fopt;
    *output = result;
  }
  
protected:
  friend class EllipsoidalFunctionClass;
  
  std::vector<double> coefs;
  double fopt;
};
  
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_TEST_BED_H_
