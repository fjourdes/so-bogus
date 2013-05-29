/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_BLOCK_GAUSS_SEIDEL_HPP
#define BOGUS_BLOCK_GAUSS_SEIDEL_HPP

#include "BlockSolverBase.hpp"

#include <vector>

namespace bogus
{

//! Projected Gauss-Seidel iterative solver. See solve() and \cite JAJ98.
template < typename BlockMatrixType >
class GaussSeidel : public BlockSolverBase< BlockMatrixType >
{
public:
	typedef BlockSolverBase< BlockMatrixType > Base ;

	typedef typename Base::LocalMatrixType LocalMatrixType ;
	typedef typename Base::GlobalProblemTraits GlobalProblemTraits ;
	typedef typename GlobalProblemTraits::Scalar Scalar ;

	//! Default constructor -- you will have to call setMatrix() before using the solve() function
	GaussSeidel( ) ;
	//! Constructor with the system matrix
	explicit GaussSeidel( const BlockMatrixBase< BlockMatrixType > & matrix ) ;

	//! Sets the system matrix and initializes internal structures
	void setMatrix( const BlockMatrixBase< BlockMatrixType > & matrix ) ;

	//! Solves a constrained linear system
	/*!
	  Implements Algorithm 1. from \cite DBB11 to solve
	   \f[
		\left\{
		  \begin{array}{rcl}
			y &=& M x + b \\
			&s.t.& law (x,y)
		  \end{array}
		\right.
	  \f]
	  \param law The (non-smooth) law that should define:
		- An error function for the global problem
		- A local solver for each row of the system ( e.g. 1 contact solver )
		\sa SOCLaw
	  \param b the const part of the right hand side
	  \param x the unknown. Can be warm-started
	  */
	template < typename NSLaw, typename RhsT, typename ResT >
	Scalar solve( const NSLaw &law, const RhsT &b, ResT &x ) const ;

	//! Sets whether the solver is allowed to trade off determiniticity for speed
	void setDeterministic( bool deterministic ) { m_deterministic = deterministic ; }


	// Debug

	//! Sets the number of iterations that should be performed between successive evaluations of the global error function
	/*! ( Those evaluations require a full matrix/vector product, and are therfore quite costly ) */
	void setEvalEvery( unsigned evalEvery ) { m_evalEvery = evalEvery ; }
	//! Sets the minimum iteration step size under which local problems are temporarily frozen
	void setSkipTol  ( unsigned skipTol   ) { m_skipTol   = skipTol   ; }
	//! Sets the number of iterations for temporarily freezing local problems
	void setSkipIters( unsigned skipIters ) { m_skipIters = skipIters ; }

protected:
	using Base::m_matrix ;
	using Base::m_maxIters ;
	using Base::m_tol ;

	typename BlockContainerTraits< LocalMatrixType >::Type m_localMatrices ;
	typename GlobalProblemTraits::DynVector m_scaling ;

	bool m_deterministic ;

	unsigned m_evalEvery ;
	Scalar m_skipTol ;
	Scalar m_skipIters ;
} ;

} //namespace bogus


#endif
