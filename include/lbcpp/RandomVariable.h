/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: RandomVariable.h               | RandomGenerator variable statistics      |
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
** @class ScalarVariable
** @brief
*/

class ScalarVariableMean : public Object
{
public:
  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  ScalarVariableMean(const String& name = "")
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
  virtual String getName() const
    {return name;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual String toString() const
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
  String name;             /*!< */
  double value;                 /*!< */
  double cnt;                   /*!< */
};


/*!
** @class ScalarVariableMeanAndVariance
** @brief
*/
class ScalarVariableMeanAndVariance : public ScalarVariableMean
{
public:
  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  ScalarVariableMeanAndVariance(const String& name = "")
    : ScalarVariableMean(name) {}

  /*!
  **
  **
  ** @param val
  */
  void push(double val)
    {ScalarVariableMean::push(val); meansqr.push(sqr(val));}

  /*!
  **
  **
  ** @param val
  ** @param weight
  */
  void push(double val, double weight)
    {ScalarVariableMean::push(val, weight); meansqr.push(sqr(val), weight);}

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
  virtual String toString() const
    {return ScalarVariableMean::toString() + " +/- " + lbcpp::toString(getStandardDeviation());}

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
  {
    ScalarVariableMean::save(ostr);
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
    {return ScalarVariableMean::load(istr) && meansqr.load(istr);}

private:
  ScalarVariableMean meansqr; /*!< */

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
** @class ScalarVariableStatics
** @brief
*/
class ScalarVariableStatistics : public ScalarVariableMeanAndVariance
{
 public:
  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  ScalarVariableStatistics(const String& name = "")
    : ScalarVariableMeanAndVariance(name), min(DBL_MAX), max(-DBL_MAX) {}

  /*!
  **
  **
  ** @param val
  */
  void push(double val)
  {
    ScalarVariableMeanAndVariance::push(val);
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
    ScalarVariableMeanAndVariance::push(val, weight);
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
  virtual String toString() const
  {
    return ScalarVariableMeanAndVariance::toString() + " [" +
      lbcpp::toString(min) + " - " + lbcpp::toString(max) + "]";
  }

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
  {
    ScalarVariableMeanAndVariance::save(ostr);
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
    {return ScalarVariableMeanAndVariance::load(istr) && read(istr, min) && read(istr, max);}

private:
  double min;                   /*!< */
  double max;                   /*!< */
};

class PolicyStatistics : public Object
{
public:
  PolicyStatistics();

  ScalarVariableStatisticsPtr getRewardPerChoose() const
    {return rewardPerChoose;}

  ScalarVariableStatisticsPtr getRewardPerEpisode() const
    {return rewardPerEpisode;}

  ScalarVariableStatisticsPtr getChoicesPerChoose() const
    {return choicesPerChoose;}

  ScalarVariableStatisticsPtr getChoosesPerEpisode() const
    {return choosesPerEpisode;}

  ScalarVariableStatisticsPtr getChoicesPerEpisode() const
    {return choicesPerEpisode;}

  // shortcuts
  double getRewardPerChooseMean() const
    {return rewardPerChoose->getMean();}

  double getRewardPerChooseStddev() const
    {return rewardPerChoose->getStandardDeviation();}

  double getRewardPerEpisodeMean() const
    {return rewardPerEpisode->getMean();}

  double getRewardPerEpisodeStddev() const
    {return rewardPerEpisode->getStandardDeviation();}

  // Object
  virtual String toString() const;

private:
  ScalarVariableStatisticsPtr rewardPerChoose;
  ScalarVariableStatisticsPtr choicesPerChoose;
  ScalarVariableStatisticsPtr rewardPerEpisode;
  ScalarVariableStatisticsPtr choosesPerEpisode;
  ScalarVariableStatisticsPtr choicesPerEpisode;
};

}; /* namespace lbcpp */

#endif // !LBCPP_RANDOM_VARIABLE_H_
