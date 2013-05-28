/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BOGUS_CONJUGATE_GRADIENT_HPP
#define BOGUS_CONJUGATE_GRADIENT_HPP

#include "BlockSolverBase.hpp"
#include "Preconditioners.hpp"

#include <vector>

namespace bogus
{

//! Preconditionned Conjugate Gradient and variations
/*!
  \tparam BlockMatrixT The type of system matrix, which should be a subclass of BlockMatrixBase
  \tparam PreconditionerType The preconditioner type. It should accept BlockMatrixT as a template parameter.
	The default value, TrivialPreconditioner, means that no preconditioning will be done.
	\sa TrivialPreconditioner, DiagonalPreconditioner, DiagonalLUPreconditioner, DiagonalLDLTPreconditioner
  */

template < typename BlockMatrixType,
		   template< typename BlockMatrixT > class PreconditionerType = TrivialPreconditioner >
class ConjugateGradient : public BlockSolverBase< BlockMatrixType >
{
public:
	typedef BlockSolverBase< BlockMatrixType > Base ;

	typedef typename Base::LocalMatrixType LocalMatrixType ;
	typedef typename Base::GlobalProblemTraits GlobalProblemTraits ;
	typedef typename GlobalProblemTraits::Scalar Scalar ;

	//! Default constructor -- you will have to call setMatrix() before using any of the solve() functions
	ConjugateGradient( ) ;
	//! Constructor with the system matrix -- initializes preconditioner
	explicit ConjugateGradient( const BlockMatrixBase< BlockMatrixType > & matrix ) ;

	//! Sets the system matrix and initializes the preconditioner
	void setMatrix( const BlockMatrixBase< BlockMatrixType > & matrix ) ;

	//! Solves ( m_matrix * \p x = \p b ) using the Conjugate Gradient algorithm
	/*! Works for symmetric positive definite linear systems.*/
	template < typename RhsT, typename ResT >
	Scalar solve( const RhsT &b, ResT &x ) const ;

	//! Solves ( m_matrix * \p x = \p b ) using the BiConjugate Gradient algorithm
	/*! Works for non-symmetric linear systems. Convergence not guaranteed */
	template < typename RhsT, typename ResT >
	Scalar solve_BiCG( const RhsT &b, ResT &x ) const ;

	//! Solves ( m_matrix * \p x = \p b ) using the BiConjugate Gradient stabilized algorithm
	/*! Works for non-symmetric linear systems. Convergence not guaranteed */
	template < typename RhsT, typename ResT >
	Scalar solve_BiCGSTAB( const RhsT &b, ResT &x ) const ;


protected:
	using Base::m_matrix ;
	using Base::m_maxIters ;
	using Base::m_tol ;

	PreconditionerType< BlockMatrixBase< BlockMatrixType > > m_preconditioner ;
} ;

} //namesoace bogus

#endif

