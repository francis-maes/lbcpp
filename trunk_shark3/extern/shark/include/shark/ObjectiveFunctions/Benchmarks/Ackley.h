/**
 *
 *  \brief Convex quadratic benchmark function with single dominant axis
 *
 *  <BR><HR>
 *  This file is part of Shark. This library is free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software
 *  Foundation; either version 3, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef SHARK_OBJECTIVEFUNCTIONS_BENCHMARKS_ACKLEY_H
#define SHARK_OBJECTIVEFUNCTIONS_BENCHMARKS_ACKLEY_H

#include <shark/ObjectiveFunctions/AbstractObjectiveFunction.h>
#include <shark/Core/SearchSpaces/VectorSpace.h>
#include <shark/Rng/GlobalRng.h>

namespace shark {
/**
 * \brief Convex quadratic benchmark function with single dominant axis
 */
struct Ackley : public AbstractObjectiveFunction< VectorSpace<double>,double > {
	Ackley(unsigned int numberOfVariables = 5) {
		m_features |= CAN_PROPOSE_STARTING_POINT;
		m_name="Ackley";
		m_numberOfVariables = numberOfVariables;
	}
	
	std::size_t numberOfVariables()const{
		return m_numberOfVariables;
	}
	
	bool hasScalableDimensionality()const{
		return true;
	}
	
	/// \brief Adjusts the number of variables if the function is scalable.
	/// \param [in] numberOfVariables The new dimension.
	void setNumberOfVariables( std::size_t numberOfVariables ){
		m_numberOfVariables = numberOfVariables;
	}

	void configure(const PropertyTree &node) {
		m_numberOfVariables = node.get("numberOfVariables",5l);
	}

	void proposeStartingPoint(SearchPointType &x) const {
		x.resize(m_numberOfVariables);

		for (unsigned int i = 0; i < x.size(); i++) {
			x(i) = Rng::uni(-10, 10);
		}
	}

	double eval(const SearchPointType &p) const {
		m_evaluationCounter++;

		const double A = 20.;
		const double B = 0.2;
		const double C = 2* M_PI;

		std::size_t n = p.size();
		double a = 0., b = 0.;

		for (std::size_t i = 0; i < n; ++i) {
			a += p(i) * p(i);
			b += cos(C * p(i));
		}

		return -A * std::exp(-B * std::sqrt(a / n)) - std::exp(b / n) + A + M_E;
	}
private:
	std::size_t m_numberOfVariables;
};

ANNOUNCE_SINGLE_OBJECTIVE_FUNCTION(Ackley, shark::soo::RealValuedObjectiveFunctionFactory);
}

#endif // SHARK_EA_ACKLEY_H