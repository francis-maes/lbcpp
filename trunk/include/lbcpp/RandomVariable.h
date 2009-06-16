/*-----------------------------------------.---------------------------------.
| Filename: RandomVariable.h               | Random variable statistics      |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   RandomVariable.h
**@author Francis MAES
**@date   Fri Jun 12 19:25:23 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_RANDOM_VARIABLE_H_
# define LBCPP_RANDOM_VARIABLE_H_

# include "ObjectPredeclarations.h"
# include <cfloat>

namespace lbcpp
{

/*!
** @class ScalarRandomVariable
** @brief
*/

class ScalarRandomVariableMean : public Object
{
public:
  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  ScalarRandomVariableMean(const std::string& name = "")
    : name(name), value(0), cnt(0) {}

  /*!
  **
  **
  ** @param val
  */
  void push(double val)
    {value = (value * cnt + val) / (cnt + 1); ++cnt;}

  /*!
  **
  **
  ** @param val
  ** @param weight
  */
  void push(double val, double weight)
    {value = (value * cnt + val) / (cnt + weight); cnt += weight;}

  /*!
  **
  **
  **
  ** @return
  */
  double getMean() const
    {return value;}

  /*!
  **
  **
  **
  ** @return
  */
  double getCount() const
    {return cnt;}

  /*!
  **
  **
  **
  ** @return
  */
  double getSum() const
    {return value * cnt;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string getName() const
    {return name;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const
    {return lbcpp::toString(getMean());}

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
  {
    write(ostr, name);
    write(ostr, value);
    write(ostr, cnt);
  }

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr)
    {return read(istr, name) && read(istr, value) && read(istr, cnt);}

protected:
  std::string name;             /*!< */
  double value;                 /*!< */
  double cnt;                   /*!< */
};


/*!
** @class ScalarRandomVariableMeanAndVariance
** @brief
*/
class ScalarRandomVariableMeanAndVariance : public ScalarRandomVariableMean
{
public:
  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  ScalarRandomVariableMeanAndVariance(const std::string& name = "")
    : ScalarRandomVariableMean(name) {}

  /*!
  **
  **
  ** @param val
  */
  void push(double val)
    {ScalarRandomVariableMean::push(val); meansqr.push(sqr(val));}

  /*!
  **
  **
  ** @param val
  ** @param weight
  */
  void push(double val, double weight)
    {ScalarRandomVariableMean::push(val, weight); meansqr.push(sqr(val), weight);}

  /*!
  **
  **
  **
  ** @return
  */
  double getVariance() const
    {return meansqr.getMean() - sqr(getMean());}

  /*!
  **
  **
  **
  ** @return
  */
  double getStandardDeviation() const
    {double v = getVariance(); return v > DBL_EPSILON ? sqrt(v) : 0.0;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const
    {return ScalarRandomVariableMean::toString() + " +/- " + lbcpp::toString(getStandardDeviation());}

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
  {
    ScalarRandomVariableMean::save(ostr);
    meansqr.save(ostr);
  }

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr)
    {return ScalarRandomVariableMean::load(istr) && meansqr.load(istr);}

private:
  ScalarRandomVariableMean meansqr; /*!< */

  /*!
  **
  **
  ** @param x
  **
  ** @return
  */
  static inline double sqr(double x)
    {return x * x;}
};

/*!
** @class ScalarRandomVariableStatics
** @brief
*/
class ScalarRandomVariableStatistics : public ScalarRandomVariableMeanAndVariance
{
 public:
  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  ScalarRandomVariableStatistics(const std::string& name = "")
    : ScalarRandomVariableMeanAndVariance(name), min(DBL_MAX), max(-DBL_MAX) {}

  /*!
  **
  **
  ** @param val
  */
  void push(double val)
  {
    ScalarRandomVariableMeanAndVariance::push(val);
    if (val < min)
	    min = val;
    if (val > max)
	    max = val;
  }

  /*!
  **
  **
  ** @param val
  ** @param weight
  */
  void push(double val, double weight)
  {
    ScalarRandomVariableMeanAndVariance::push(val, weight);
    if (val < min)
	    min = val;
    if (val > max)
	    max = val;
  }

  /*!
  **
  **
  **
  ** @return
  */
  double getMinimum() const
    {return min;}

  /*!
  **
  **
  **
  ** @return
  */
  double getMaximum() const
    {return max;}

  /*!
  **
  **
  **
  ** @return
  */
  double getRange() const
    {return max - min;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual std::string toString() const
  {
    return ScalarRandomVariableMeanAndVariance::toString() + " [" +
      lbcpp::toString(min) + " - " + lbcpp::toString(max) + "]";
  }

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
  {
    ScalarRandomVariableMeanAndVariance::save(ostr);
    write(ostr, min);
    write(ostr, max);
  }

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr)
    {return ScalarRandomVariableMeanAndVariance::load(istr) && read(istr, min) && read(istr, max);}

private:
  double min;                   /*!< */
  double max;                   /*!< */
};

}; /* namespace lbcpp */

#endif // !LBCPP_RANDOM_VARIABLE_H_
