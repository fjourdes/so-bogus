/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver          
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>                       
 *
 * This Source Code Form is subject to the terms of the Mozilla Public 
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_NS_NETWON_IMPL_HPP
#define BOGUS_NS_NETWON_IMPL_HPP

#include "NonSmoothNewton.hpp"

#include "NumTraits.hpp"
#include "LinearSolverBase.hpp"

#include <iostream>

namespace bogus {

template < typename NSFunction >
typename NonSmoothNewton< NSFunction >::Scalar NonSmoothNewton<NSFunction>::solve(
  Vector& x ) const
{
	// see [Daviet et al 2011], Appendix A.2

  static const Scalar sigma2 = 1.e-4 ;
  static const Scalar alpha = .5 ;

  Vector F ;
  m_func.compute( x, F ) ;
  const Scalar Phi_init = F.squaredNorm() ;

  if( Phi_init < m_tol ) return Phi_init ;

  Scalar Phi_best ;
  Vector x_best = Vector::Zero() ;

  m_func.compute( x_best, F ) ;
  const Scalar Phi_zero = F.squaredNorm() ;

  if( Phi_zero < Phi_init ) {
	Phi_best = Phi_zero ;
	x = x_best ;
  } else {
	Phi_best = Phi_init ;
	x_best = x ;
  }

  if( Phi_zero < m_tol ) return Phi_zero ;

  Matrix dF_dx ;
  Vector dPhi_dx, dx ;

  typename Traits::LUType lu ;

  for( unsigned iter = 0 ; iter < m_maxIters ; ++iter )
  {
	m_func.computeJacobian( x, F, dF_dx ) ;
	const Scalar Phi = F.squaredNorm() ;

	if( Phi < m_tol ) return Phi ;
	if( Phi < Phi_best ) {
	  Phi_best = Phi ;
	  x_best = x ;
	}

	dPhi_dx = dF_dx.transpose() * x ;

	dx = - lu.compute( dF_dx ).solve( F ) ;
	const Scalar proj = dx.dot( dPhi_dx ) ;

	if( proj > 0 || proj * proj < sigma2 * dx.squaredNorm() * dPhi_dx.squaredNorm() )
	{
		dx *= alpha ;
	}

	x += dx ;

  }

  x = x_best ;
  return Phi_best ;

}

}


#endif
