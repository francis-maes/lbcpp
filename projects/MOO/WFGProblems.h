/*-----------------------------------------.---------------------------------.
| Filename: WFGProblems.h                  | The WFG benchmark suite         |
| Author  : Denny Verbeeck                 |                                 |
| Started : 14/03/2014 17:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef MOO_PROBLEM_WFG_H_
# define MOO_PROBLEM_WFG_H_

# include <ml/Problem.h>
# include <ml/Domain.h>

namespace lbcpp
{

struct WFGShapes
{
  static double linear(DenseDoubleVectorPtr x, size_t m)
  {
    double result = 1.0;
    size_t numDims = x->getNumValues();

    for (size_t i = 1; i <= numDims - m; ++i)
      result *= x->getValue(i - 1);

    if (m != 1)
      result *= (1 - x->getValue(numDims - m));

    return result;
  }

  static double convex(DenseDoubleVectorPtr x, size_t m)
  {
    double result = 1.0;
    size_t numDims = x->getNumValues();

    for (size_t i = 1; i <= numDims - m; ++i)
      result *= (1 - cos(x->getValue(i - 1) * M_PI_2));

    if (m != 1)
      result *= (1 - sin(x->getValue(numDims - m) * M_PI_2));

    return result;
  }

  static double concave(DenseDoubleVectorPtr x, size_t m)
  {
    double result = 1.0;
    size_t numDims = x->getNumValues();

    for (size_t i = 1; i <= numDims - m; ++i)
      result *= sin(x->getValue(i - 1) * M_PI_2);

    if (m != 1)
      result *= cos(x->getValue(numDims - m) * M_PI_2);

    return result;
  }

  static double mixed(DenseDoubleVectorPtr x, size_t A, double alpha)
  {
    double tmp = cos(A * M_2PI * x->getValue(0) + M_PI_2) / (A * M_2PI);
    return pow(1.0 - x->getValue(0) - tmp, alpha);
  }

  static double disc(DenseDoubleVectorPtr x, size_t A, double alpha, double beta)
  {
    double tmp = cos(A * pow(x->getValue(0), beta) * M_PI);
    return (1.0 - pow(x->getValue(0), alpha) * tmp * tmp);
  }
};
struct WFGTransformations
{
  static double b_poly(double y, double alpha)
    {jassert(alpha > 0); return clamp(pow(y, alpha), 0.0, 1.0);}

  static double b_flat(double y, double A, double B, double C)
  {
    double tmp1 = min(0.0, floor(y - B)) * A * (B - y) / B;
    double tmp2 = min(0.0, floor(C - y)) * (1 - A) * (y - C) / (1 - C);
    return clamp(A + tmp1 - tmp2, 0.0, 1.0);
  }

  static double s_linear(double y, double A)
    {return clamp(abs(y - A) / abs(floor(A - y) + A), 0.0, 1.0);}

  static double s_decept(double y, double A, double B, double C)
  {
    double tmp1 = floor(y - A + B) * (1.0 - C + (A - B) / B) / (A - B);
    double tmp2 = floor(A + B - y) * (1.0 - C + (1.0 - A - B) / B) / (1.0 - A - B);
    double tmp = abs(y - A) - B;
    return clamp(1.0 + tmp * (tmp1 + tmp2 + 1.0 / B), 0.0, 1.0);
  }

  static double s_multi(double y, size_t A, size_t B, double C)
  {
    double tmp1 = (4.0 * A + 2.0) * M_PI * (0.5 - abs(y - C) / (2.0 * (floor(C - y) + C)));
    double tmp2 = 4.0 * B * pow(abs(y - C) / (2.0 * (floor(C - y) + C)), 2.0);
    return clamp((1.0 + cos(tmp1) + tmp2) / (B + 2.0), 0.0, 1.0);
  }

  static double r_sum(DenseDoubleVectorPtr y, DenseDoubleVectorPtr w)
  {
    double tmp1 = 0.0;
    double tmp2 = 0.0;
    for (size_t i = 0; i < y->getNumValues(); ++i)
    {
      tmp1 += y->getValue(i) * w->getValue(i);
      tmp2 += w->getValue(i);
    }
    return clamp(tmp1 / tmp2, 0.0, 1.0);
  }

  static double r_nonsep(DenseDoubleVectorPtr y, size_t A)
  {
    size_t n = y->getNumValues();
    double tmp = ceil(A / 2.0);
    double denominator = n * tmp * (1.0 + 2.0 * (A - tmp)) / A;
    double numerator = 0.0;
    for (size_t j = 0; j < n; ++j)
    {
      numerator += y->getValue(j);
      for (size_t k = 0; k <= A-2; ++k)
        numerator += abs(y->getValue(j) - y->getValue((j + k + 1) % n));
    }
    return clamp(numerator / denominator, 0.0, 1.0);
  }

  static double b_param(double y, double u, double A, double B, double C)
  {
    double v = A - (1.0 - 2.0 * u) * abs(floor(0.5 - u) + A);
    double exp = B + (C - B) * v;
    return clamp(pow(y, exp), 0.0, 1.0);
  }

  static inline double clamp(double y, double min, double max)
    {return y < min ? min : (y > max ? max : y);}
};

class WFGObjective : public Objective
{
public:
  WFGObjective(size_t k, size_t l, size_t m, size_t idx) : k(k), l(l), m(m), idx(idx), 
               d(1), a(std::vector<size_t>(m - 1, 0)), s(std::vector<size_t>(m, 0))
  {
    for (size_t i = 0; i < m; ++i)
      s[i] = 2 * (i + 1);
  }

protected:
  size_t k;             //!< number of position-related parameters
  size_t l;             //!< number of distance-related parameters
  size_t m;             //!< number of objectives
  size_t idx;           //!< index of this objective in the set of objectives
  std::vector<size_t> a;
  std::vector<size_t> s;
  size_t d;

  DenseDoubleVectorPtr calculateX(const DenseDoubleVectorPtr& t) const
  {
    DenseDoubleVectorPtr x = new DenseDoubleVector(m, 0.0);
    for (size_t i = 0; i < m - 1; ++i)
      x->setValue(i, std::max(t->getValue(m - 1), (double)a[i]) * (t->getValue(i) - 0.5) + 0.5);
    x->setValue(m-1, t->getValue(m-1));
    return x;
  }

  DenseDoubleVectorPtr normalize(const DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(z->getNumValues(), 0.0);
    for (size_t i = 0; i < z->getNumValues(); ++i)
    {
      double norm = z->getValue(i) / (2.0 * (i + 1));
      result->setValue(i, WFGTransformations::clamp(norm, 0.0, 1.0));
    }
    return result;
  }

  DenseDoubleVectorPtr subVector(const DenseDoubleVectorPtr& original, size_t start, size_t end) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(end - start + 1, 0.0);
    for (size_t i = start; i <= end; ++i)
      result->setValue(i - start, original->getValue(i));
    return result;
  }
};

class WFG1Objective : public WFGObjective
{
public:
  WFG1Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    t3(y);
    t4(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m - 1)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::convex(x, idx + 1);
    else if (idx == m - 1)
      return d * x->getValue(m - 1) + s[m - 1] * WFGShapes::mixed(x, 5, 1.0);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 4.0; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
    {for (size_t i = k; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_linear(z->getValue(i), 0.35));}

  void t2(DenseDoubleVectorPtr& z) const
    {for (size_t i = k; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::b_flat(z->getValue(i), 0.8, 0.75, 0.85));}

  void t3(DenseDoubleVectorPtr& z) const
    {for (size_t i = 0; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::b_poly(z->getValue(i), 0.02));}

  void t4(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 0.0);
    
    for (size_t i = 0; i < z->getNumValues(); ++i)
      w->setValue(i, 2.0 * (i + 1));

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    }

    size_t head = k + 1 - 1;
    size_t tail = z->getNumValues() - 1;
    result->setValue(m - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    z = result;
  }
};

class WFG2Objective : public WFGObjective
{
public:
  WFG2Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    t3(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m - 1)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::convex(x, idx + 1);
    else if (idx == m - 1)
      return d * x->getValue(m - 1) + s[m - 1] * WFGShapes::disc(x, 5, 1.0, 1.0);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 2.4; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
    {for (size_t i = k; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_linear(z->getValue(i), 0.35));}

  void t2(DenseDoubleVectorPtr& z) const
  {
    for (size_t i = k + 1; i <= k + l / 2; ++i)
    {
      size_t head = k + 2 * (i - k) - 1;
      size_t tail = k + 2* (i - k);
      z->setValue(i - 1, WFGTransformations::r_nonsep(subVector(z, head - 1, tail - 1), 2));
    }
  }

  void t3(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    }

    size_t head = k;
    size_t tail = k + l / 2 - 1;
    result->setValue(m - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    z = result;
  }
};

class WFG3Objective : public WFGObjective
{
public:
  WFG3Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx) 
    {a[0] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    t3(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::linear(x, idx + 1);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 3.0; best = 2.5;}
    else if (idx == 1) {worst = -1.0; best = -2.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
    {for (size_t i = k; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_linear(z->getValue(i), 0.35));}

  void t2(DenseDoubleVectorPtr& z) const
  {
    for (size_t i = k + 1; i <= k + l / 2; ++i)
    {
      size_t head = k + 2 * (i - k) - 1;
      size_t tail = k + 2* (i - k);
      z->setValue(i - 1, WFGTransformations::r_nonsep(subVector(z, head - 1, tail - 1), 2));
    }
  }

  void t3(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    }

    size_t head = k;
    size_t tail = k + l / 2 - 1;
    result->setValue(m - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    z = result;
  }
};

class WFG4Objective : public WFGObjective
{
public:
  WFG4Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::convex(x, idx + 1);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 4.0; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
    {for (size_t i = 0; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_multi(z->getValue(i), 30, 10, 0.35));}

  void t2(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    }

    size_t head = k;
    size_t tail = z->getNumValues() - 1;
    result->setValue(m - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    z = result;
  }
};

class WFG5Objective : public WFGObjective
{
public:
  WFG5Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::concave(x, idx + 1);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 4.0; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
    {for (size_t i = 0; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_decept(z->getValue(i), 0.35, 0.001, 0.05));}

  void t2(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    }

    size_t head = k;
    size_t tail = z->getNumValues() - 1;
    result->setValue(m - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    z = result;
  }
};

class WFG6Objective : public WFGObjective
{
public:
  WFG6Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::concave(x, idx + 1);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 4.0; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
    {for (size_t i = k; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_linear(z->getValue(i), 0.35));}

  void t2(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_nonsep(subVector(z, head, tail), k / (m - 1)));
    }

    size_t head = k;
    size_t tail = z->getNumValues() - 1;
    result->setValue(m - 1, WFGTransformations::r_nonsep(subVector(z, head, tail), l));
    z = result;
  }
};

class WFG7Objective : public WFGObjective
{
public:
  WFG7Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    t3(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::concave(x, idx + 1);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 4.0; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(z->getNumValues(), 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 0; i < k; ++i)
    {
      size_t head = i + 1;
      size_t tail = z->getNumValues() - 1;
      double aux = WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail));
      result->setValue(i, WFGTransformations::b_param(z->getValue(i), aux, 0.98 / 49.98, 0.02, 50.0));
    }

    for (size_t i = k; i < z->getNumValues(); ++i)
      result->setValue(i, z->getValue(i));

    z = result;
  }

  void t2(DenseDoubleVectorPtr& z) const
    {for (size_t i = k; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_linear(z->getValue(i), 0.35));}

  void t3(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    }

    size_t head = k;
    size_t tail = m - 1;
    result->setValue(m - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    z = result;
  }
};

class WFG8Objective : public WFGObjective
{
public:
  WFG8Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    t3(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::concave(x, idx + 1);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 4.0; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(z->getNumValues(), 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 0; i < k; ++i)
      result->setValue(i, z->getValue(i));

    for (size_t i = k; i < z->getNumValues(); ++i)
    {
      size_t head = 0;
      size_t tail = i - 1;
      double aux = WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail));
      result->setValue(i, WFGTransformations::b_param(z->getValue(i), aux, 0.98 / 49.98, 0.02, 50.0));
    }
    
    z = result;
  }

  void t2(DenseDoubleVectorPtr& z) const
    {for (size_t i = k; i < z->getNumValues(); ++i) z->setValue(i, WFGTransformations::s_linear(z->getValue(i), 0.35));}

  void t3(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    }

    size_t head = k;
    size_t tail = m - 1;
    result->setValue(m - 1, WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail)));
    z = result;
  }
};

class WFG9Objective : public WFGObjective
{
public:
  WFG9Objective(size_t k, size_t l, size_t m, size_t idx) : WFGObjective(k, l, m, idx)
    {for (size_t i = 0; i < m - 1; ++i) a[i] = 1;}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) const
  {
    DenseDoubleVectorPtr y = normalize(object.staticCast<DenseDoubleVector>());
    t1(y);
    t2(y);
    t3(y);
    DenseDoubleVectorPtr x = calculateX(y);
    if (idx >= 0 && idx < m)
      return d * x->getValue(m - 1) + s[idx] * WFGShapes::concave(x, idx + 1);
    jassertfalse;
    return 0.0;
  }

  virtual void getObjectiveRange(double& worst, double& best) const
  {
    if (idx == 0) {worst = 2.0; best = 0.0;}
    else if (idx == 1) {worst = 4.0; best = 0.0;}
    else jassertfalse;
  }

protected:
  void t1(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(z->getNumValues(), 0.0);
    DenseDoubleVectorPtr w = new DenseDoubleVector(z->getNumValues(), 1.0);

    for (size_t i = 0; i < z->getNumValues() - 1; ++i)
    {
      size_t head = i + 1;
      size_t tail = z->getNumValues() - 1;
      double aux = WFGTransformations::r_sum(subVector(z, head, tail), subVector(w, head, tail));
      result->setValue(i, WFGTransformations::b_param(z->getValue(i), aux, 0.98 / 49.98, 0.02, 50.0));
    }
    
    z = result;
  }

  void t2(DenseDoubleVectorPtr& z) const
  {
    for (size_t i = 0; i < k; ++i)
      z->setValue(i, WFGTransformations::s_decept(z->getValue(i), 0.35, 0.001, 0.05));
    for (size_t i = k; i < m; ++i)
      z->setValue(i, WFGTransformations::s_multi(z->getValue(i), 30, 95, 0.35));
  }

  void t3(DenseDoubleVectorPtr& z) const
  {
    DenseDoubleVectorPtr result = new DenseDoubleVector(m, 0.0);

    for (size_t i = 1; i < m; ++i)
    {
      size_t head = (i - 1) * k / (m - 1);
      size_t tail = i * k / (m - 1) - 1;
      result->setValue(i - 1, WFGTransformations::r_nonsep(subVector(z, head, tail), k / (m - 1)));
    }

    size_t head = k;
    size_t tail = z->getNumValues() - 1;
    result->setValue(m - 1, WFGTransformations::r_nonsep(subVector(z, head, tail), l));
    z = result;
  }
};

class WFGProblem : public Problem
{
public:
  WFGProblem(size_t posParams, size_t distanceParams, size_t numObjectives) : k(posParams), l(distanceParams), m(numObjectives)
    {initialize(defaultExecutionContext());}

protected:
  size_t k;             //!< number of position-related parameters
  size_t l;             //!< number of distance-related parameters
  size_t m;             //!< number of objectives

  virtual void initialize(ExecutionContext& context)
  {
    // setup domain
    std::vector<std::pair<double, double> > inputdomain;
    for (size_t i = 0; i < k + l; ++i)
      inputdomain.push_back(std::make_pair(0, 2 * (i + 1)));
    domain = new ScalarVectorDomain(inputdomain);
  }
};

class WFG1Problem : public WFGProblem
{
public:
  WFG1Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG1Objective(k, l, m, i));
  }
};

class WFG2Problem : public WFGProblem
{
public:
  WFG2Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG2Objective(k, l, m, i));
  }
};

class WFG3Problem : public WFGProblem
{
public:
  WFG3Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG3Objective(k, l, m, i));
  }
};

class WFG4Problem : public WFGProblem
{
public:
  WFG4Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG4Objective(k, l, m, i));
  }
};

class WFG5Problem : public WFGProblem
{
public:
  WFG5Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG5Objective(k, l, m, i));
  }
};

class WFG6Problem : public WFGProblem
{
public:
  WFG6Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG6Objective(k, l, m, i));
  }
};

class WFG7Problem : public WFGProblem
{
public:
  WFG7Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG7Objective(k, l, m, i));
  }
};

class WFG8Problem : public WFGProblem
{
public:
  WFG8Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG8Objective(k, l, m, i));
  }
};

class WFG9Problem : public WFGProblem
{
public:
  WFG9Problem(size_t posParams = 4, size_t distanceParams = 20, size_t numObjectives = 2) : WFGProblem(posParams, distanceParams, numObjectives) 
  {
    for (size_t i = 0; i < m; ++i)
      addObjective(new WFG9Objective(k, l, m, i));
  }
};


} /* namespace lbcpp */


#endif //!MOO_PROBLEM_WFG_H_
