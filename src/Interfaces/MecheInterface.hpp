/*
 * This file is part of So-bogus, a C++ sparse block matrix library and
 * Second Order Cone solver.
 *
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * So-bogus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.

 * So-bogus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with So-bogus.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iosfwd>

#include "../Core/Utils/Signal.hpp"
#include "../Core/Utils/Timer.hpp"
#include "../Core/Utils/CppTools.hpp"

#ifndef BOGUS_MECHE_INTERFACE_HPP
#define BOGUS_MECHE_INTERFACE_HPP

namespace bogus
{

template< unsigned Dimension > struct PrimalFrictionProblem ;
template< unsigned Dimension > struct DualFrictionProblem ;

class MecheFrictionProblem
{
public:

	MecheFrictionProblem() ;
	~MecheFrictionProblem() ;

	//! Allocates and sets up the primal friction problem
	/*! \warning copies the contents of the matrices M, E and H !
		Manually constructing a PrimalFrictionProblem would be more efficient
	*/
	void fromPrimal (
			unsigned int NObj, //!< number of subsystems
			const unsigned int * ndof, //!< array of size \a NObj, the number of degree of freedom of each subsystem
			const double *const * MassMat, //!< array of pointers to the mass matrix of each subsystem
			const double * f_in, //!< the constant term in \f$ M v + f= {}^t \! H r \f$
			unsigned int n_in, //!< number of contact points
			const double * mu_in, //!< array of size \a n giving the friction coeffs
			const double * E_in, //!< array of size \f$ n \times d \times d \f$ giving the \a n normals followed by the \a n tangent vectors (and by again \a n tangent vectors if \a d is 3). Said otherwise, \a E is a \f$ (nd) \times d \f$ matrix, stored column-major, formed by \a n blocks of size \f$ d \times d \f$ with each block being an orthogonal matrix (the transition matrix from the world space coordinates \f$ (x_1, x_2, x_3) \f$ to the local coordinates \f$ (x_N, x_{T1}, x_{T2}) \f$
			const double * w_in, //!< array of size \a nd, the constant term in \f$ u = H v + w \f$
			const int * const ObjA, //!< array of size \a n, the first object involved in the \a i-th contact (must be an internal object) (counted from 0)
			const int * const ObjB, //!< array of size \a n, the second object involved in the \a i-th contact (-1 for an external object) (counted from 0)
			const double *const HA[], //!< array of size \a n, containing pointers to a dense, colum-major matrix of size <c> d*ndof[ObjA[i]] </c> corresponding to the H-matrix of <c> ObjA[i] </c>
			const double *const HB[] //!< array of size \a n, containing pointers to a dense, colum-major matrix of size <c> d*ndof[ObjA[i]] </c> corresponding to the H-matrix of <c> ObjB[i] </c> (\c NULL for an external object)
			);

	//! Solves the friction problem ; \sa GaussSeidel
	double solve(double * r, //!< length \a nd : initialization for \a r (in world space coordinates) + used to return computed r
			double * v, //!< length \a m: to return computed v ( or NULL if not needed )
			int maxThreads = 0,       //!< Maximum number of threads that the GS will use. If 0, use OpenMP default. If > 1, enable coloring to ensure deterministicity
			double tol = 0.,                  //!< Gauss-Seidel tolerance. 0. means GS's default
			unsigned maxIters = 0,            //!< Max number of iterations. 0 means GS's default
			bool staticProblem = false,       //!< If true, do not use DeSaxce change of variable
			double regularization = 0.,  //!< Coefficient to add to the diagonal of static problems / GS regularization coefficient for friction problems
			bool useInfinityNorm = false, //!< Whether to use the infinity norm to evaluate the residual of the friction problem,
			bool useProjectedGradient = false, //!< If true, use projected gradient algorithm instead of GS. Require either staticProblem=true, or cadouxIters > 0
			unsigned cadouxIters = 0 //!< If staticProblem is false and cadouxIters is greater than zero, use the Cadoux algorithm to solve the friction problem.
				 );

	//! Computes the dual from the primal
	void computeDual( double regularization ) ;

	//! Cleams up the problem, then allocates a new PrimalFrictionProblem and make m_primal point to it
	void reset() ;

	unsigned nDegreesOfFreedom() const ;
	unsigned nContacts() const ;

	//! Sets the standard output stream ( \p out can be NULL to remove all output )
	void setOutStream( std::ostream *out ) ;

	//! Signal< interationNumber, error, elapsedTime > that will be triggered every few iterations
	Signal< unsigned, double, double > &callback() { return m_callback ; }

	//! Dumps the current primal() to \p fileName
	/*! \param r0 The initial guess that shouls be saved with the problem, or NULL */
	bool dumpToFile( const char* fileName, const double *r0 = BOGUS_NULL_PTR(const double) ) const ;
	//! Loads the primal from a previously saved problem file
	/*! \param r0 Will be set to ploint to a newly allocated array containing the initial
		guess, if such one was saved with the problem. Will have to be manually freed
		by the caller using the delete[] operator.
	*/
	bool fromFile( const char* fileName, double* &r0 ) ;

	// solvers Callback
	void ackCurrentResidual( unsigned GSIter, double err ) ;

	// Accessors

	const PrimalFrictionProblem<3u> & primal() const { return *m_primal ; }
	const DualFrictionProblem<3u> & dual() const { return *m_dual ; }

	PrimalFrictionProblem<3u> & primal() { return *m_primal ; }
	DualFrictionProblem<3u> & dual() { return *m_dual ; }

	double *f (){ return m_f  ; }
	double *w (){ return m_w  ; }
	double *mu(){ return m_mu ; }

	//! Time spent in last solver call. In seconds.
	double lastSolveTime() const { return m_lastSolveTime ; }

protected:

	void destroy() ;

	PrimalFrictionProblem<3u> * m_primal ;
	DualFrictionProblem<3u>  * m_dual ;

	double m_lastSolveTime ;

	Signal< unsigned, double, double > m_callback ;
	Timer m_timer ;

private:
	// Non copyable, non assignable
	MecheFrictionProblem( const MecheFrictionProblem & ) ;
	MecheFrictionProblem &operator=( const MecheFrictionProblem & ) ;

	// Used to store data when loading problem from file
	double *m_f ;
	double *m_w ;
	double *m_mu ;

	std::ostream *m_out ;
} ;

}

#endif
