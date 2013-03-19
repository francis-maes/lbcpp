//===========================================================================
/*!
* \brief Multi-objective optimization benchmark function ZDT3
*
*  The function is described in
*
*  Eckart Zitzler, Kalyanmoy Deb, and Lothar Thiele. Comparison of
*  Multiobjective Evolutionary Algorithms: Empirical
*  Results. Evolutionary Computation 8(2):173-195, 2000
*
* <BR><HR>
* This file is part of Shark. This library is free software;
* you can redistribute it and/or modify it under the terms of the
* GNU General Public License as published by the Free Software
* Foundation; either version 3, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this library; if not, see <http://www.gnu.org/licenses/>.
* 
*/
//===========================================================================

#ifndef SHARK_OBJECTIVEFUNCTIONS_BENCHMARK_ZDT3_H
#define SHARK_OBJECTIVEFUNCTIONS_BENCHMARK_ZDT3_H

#include <shark/ObjectiveFunctions/AbstractMultiObjectiveFunction.h>
#include <shark/ObjectiveFunctions/BoxConstraintHandler.h>
#include <shark/Core/SearchSpaces/VectorSpace.h>

namespace shark {
/*! \brief Multi-objective optimization benchmark function ZDT3
*
*  The function is described in
*
*  Eckart Zitzler, Kalyanmoy Deb, and Lothar Thiele. Comparison of
*  Multiobjective Evolutionary Algorithms: Empirical
*  Results. Evolutionary Computation 8(2):173-195, 2000
*/
struct ZDT3 : public AbstractMultiObjectiveFunction< VectorSpace<double> >
{
	typedef AbstractMultiObjectiveFunction< VectorSpace<double> > super;

	ZDT3(std::size_t numVariables = 0) : super( 2 ) , m_handler(numVariables,0,1){
		m_features |= CAN_PROPOSE_STARTING_POINT;
		m_features |= IS_CONSTRAINED_FEATURE;
		m_features |= CAN_PROVIDE_CLOSEST_FEASIBLE;
		m_features |= HAS_CONSTRAINT_HANDLER;
		m_name="ZDT3";
	}
	
	std::size_t numberOfVariables()const{
		return m_handler.dimensions();
	}
	
	bool hasScalableDimensionality()const{
		return true;
	}

	/// \brief Adjusts the number of variables if the function is scalable.
	/// \param [in] numberOfVariables The new dimension.
	void setNumberOfVariables( std::size_t numberOfVariables ){
		m_handler.setBounds(numberOfVariables,0,1);
	}
	
	BoxConstraintHandler<SearchPointType> const& getConstraintHandler()const{
		return m_handler;
	}

	ResultType eval( const SearchPointType & x ) const {
		m_evaluationCounter++;

		ResultType value( 2 );

		value[0] = x( 0 );

		double g, h, sum = 0.0;

		for (unsigned i = 1; i < numberOfVariables(); i++)
			sum += x(i);

		sum /= (numberOfVariables() - 1.0);

		g = 1.0 + (9.0 * sum);
		h = 1.0 - ::sqrt(x(0) / g) - (x(0) / g) * ::sin(10 * M_PI * x(0));

		value[1] = g * h;

		return value;
	}

private:
	BoxConstraintHandler<SearchPointType> m_handler;
};

ANNOUNCE_MULTI_OBJECTIVE_FUNCTION( ZDT3, shark::moo::RealValuedObjectiveFunctionFactory );
}
#endif
