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

#include "FrictionProblem.hpp"

#include "../Core/Block.impl.hpp"
#include "../Core/BlockSolvers/GaussSeidel.impl.hpp"
#include "../Core/BlockSolvers/ProjectedGradient.impl.hpp"

#include "../Extra/SecondOrder.impl.hpp"

namespace bogus {

template< unsigned Dimension >
void DualFrictionProblem< Dimension >::computeFrom(PrimalFrictionProblem<Dimension> &primal )
{

	// M^-1
	primal.MInv.cloneStructure( primal.M ) ;
#ifndef BOGUS_DONT_PARALLELIZE
#pragma omp parallel for
#endif
	for( std::ptrdiff_t i = 0 ; i < (std::ptrdiff_t) primal.M.nBlocks()  ; ++ i )
	{
		primal.MInv.block(i).compute( primal.M.block(i) ) ;
	}

	//W
	W = primal.H * ( primal.MInv * primal.H.transpose() ) ;

	// M^-1 f, b
	b = primal.E.transpose() * Eigen::VectorXd::Map( primal.w, primal.H.rows())
			- primal.H * ( primal.MInv * Eigen::VectorXd::Map( primal.f, primal.H.cols() ) );

	mu = Eigen::VectorXd::Map( primal.mu, W.rowsOfBlocks() ) ;
}
namespace fp_impl {

template< unsigned Dimension, typename EigenDerived, typename Index >
void applyPermutation(
		const std::vector< std::size_t >& permutation,
		Eigen::MatrixBase< EigenDerived > &vec,
		const Index* offsets
		)
{
	Segmenter< Dimension, EigenDerived, Index > segmenter( vec.derived(), offsets ) ;
	bogus::applyPermutation( permutation.size(), &permutation[0], segmenter ) ;
}

template< unsigned Dimension, template <typename> class Method >
static double solveCadoux( const DualFrictionProblem< Dimension >& dual,
		ConstrainedSolverBase< Method, typename DualFrictionProblem< Dimension >::WType > &gs,
		double *r, const unsigned cadouxIterations, const Signal<unsigned, double> *callback )
{
	const std::ptrdiff_t n = dual.W.rowsOfBlocks() ;

	typename DualFrictionProblem< Dimension >::CoulombLawType coulombLaw( n, dual.mu.data() ) ;
	typename DualFrictionProblem< Dimension >::SOCLawType         socLaw( n, dual.mu.data() ) ;

	gs.setMatrix( dual.W );
	Eigen::Map< Eigen::VectorXd > r_map ( r, dual.W.rows() ) ;

	if( dual.permuted() )
		fp_impl::applyPermutation< Dimension >( dual.permutation(), r_map, dual.W.majorIndex().innerOffsetsData() ) ;

	Eigen::VectorXd s( dual.W.rows() ) ;

	double res = -1 ;
	const double tol = gs.tol() ;
	gs.setTol( 1.e-1 * tol ) ;	//dual.We might experience slow convergence is GS not precise enough

	for( unsigned cdxIter = 0 ; cdxIter < cadouxIterations ; ++cdxIter )
	{
		s = dual.W * r_map + dual.b ;

		res = gs.eval( coulombLaw, s, r_map ) ;

		if( callback ) callback->trigger( cdxIter, res ) ;
		if( cdxIter > 0 && res < tol ) break ;

#ifndef BOGUS_DONT_PARALLELIZE
#pragma omp parallel for
#endif
		for( std::ptrdiff_t i = 0 ; i < n ; ++i )
		{
			s[ Dimension*i ] = s.segment< Dimension-1 >( Dimension*i+1 ).norm() * dual.mu[i] ;
			s.segment< Dimension-1  >( Dimension*i+1 ).setZero() ;
		}

		s += dual.b ;

		gs.solve( socLaw, s, r_map ) ;

	}

	gs.setTol( tol ) ;

	if( dual.permuted() )
		fp_impl::applyPermutation< Dimension >( dual.invPermutation(), r_map, dual.W.majorIndex().innerOffsetsData() ) ;

	return res ;
}

} //namespace fp_impl

template< unsigned Dimension >
double DualFrictionProblem< Dimension >::solveWith( GaussSeidelType &gs, double *r,
									   const bool staticProblem ) const
{
	gs.setMatrix( W );
	typename Eigen::VectorXd::MapType r_map ( r, W.rows() ) ;

	if( permuted())
		fp_impl::applyPermutation< Dimension >( m_permutation, r_map, W.majorIndex().innerOffsetsData() ) ;

	double res = staticProblem
			? gs.solve( SOCLawType     ( W.rowsOfBlocks(), mu.data() ), b, r_map )
			: gs.solve( CoulombLawType ( W.rowsOfBlocks(), mu.data() ), b, r_map ) ;

	if( permuted())
		fp_impl::applyPermutation< Dimension >( m_invPermutation, r_map, W.majorIndex().innerOffsetsData() ) ;

	return res ;
}

template< unsigned Dimension >
double DualFrictionProblem< Dimension >::solveWith( ProjectedGradientType &pg,
													double *r ) const
{
	pg.setMatrix( W );
	typename Eigen::VectorXd::MapType r_map ( r, W.rows() ) ;

	if( permuted())
		fp_impl::applyPermutation< Dimension >( m_permutation, r_map, W.majorIndex().innerOffsetsData() ) ;

	const double res = pg.solve( SOCLawType ( W.rowsOfBlocks(), mu.data() ), b, r_map ) ;

	if( permuted())
		fp_impl::applyPermutation< Dimension >( m_invPermutation, r_map, W.majorIndex().innerOffsetsData() ) ;

	return res ;
}

template< unsigned Dimension >
double DualFrictionProblem< Dimension >::evalWith( const GaussSeidelType &gs,
											   const double *r_data,
									   const bool staticProblem ) const
{
	Eigen::VectorXd r = Eigen::VectorXd::Map( r_data, W.rows() ) ;
	Eigen::VectorXd u = W*r + b ;

	if( permuted())
		fp_impl::applyPermutation< Dimension >( m_permutation, r, W.majorIndex().innerOffsetsData() ) ;


	double res = staticProblem
			? gs.eval( SOCLawType     ( W.rowsOfBlocks(), mu.data() ), u, r )
			: gs.eval( CoulombLawType ( W.rowsOfBlocks(), mu.data() ), u, r ) ;

	return res ;
}


template< unsigned Dimension >
double DualFrictionProblem< Dimension >::solveCadoux(GaussSeidelType &gs, double *r, const unsigned cadouxIterations,
		const Signal<unsigned, double> *callback ) const
{
	return fp_impl::solveCadoux( *this, gs, r, cadouxIterations, callback ) ;
}

template< unsigned Dimension >
double DualFrictionProblem< Dimension >::solveCadoux(ProjectedGradientType &pg, double *r, const unsigned cadouxIterations,
		const Signal<unsigned, double> *callback ) const
{
	return fp_impl::solveCadoux( *this, pg, r, cadouxIterations, callback ) ;
}

template< unsigned Dimension >
void DualFrictionProblem< Dimension >::applyPermutation(
		const std::vector<std::size_t> &permutation)
{
	assert( !permuted() ) ;

	m_permutation = permutation ;

	m_invPermutation.resize( m_permutation.size() );
	for( std::size_t i = 0 ; i < m_permutation.size() ; ++i )
		m_invPermutation[ m_permutation[i] ] = i ;

	W.applyPermutation( &m_permutation[0] ) ;
	fp_impl::applyPermutation< Dimension >( m_permutation, b, W.majorIndex().innerOffsetsData() ) ;
	bogus::applyPermutation( m_permutation.size(), &m_permutation[0], mu ) ;
}

template< unsigned Dimension >
void DualFrictionProblem< Dimension >::undoPermutation()
{
	if( !permuted() )
		return ;

	W.applyPermutation( &m_invPermutation[0] ) ;
	fp_impl::applyPermutation< Dimension >( m_invPermutation, b, W.majorIndex().innerOffsetsData() ) ;
	bogus::applyPermutation( m_invPermutation.size(), &m_invPermutation[0], mu ) ;

	m_permutation.clear() ;
}

#ifdef BOGUS_INSTANTIATE_2D_SOC
template struct DualFrictionProblem< 2u > ;
template struct PrimalFrictionProblem< 2u > ;
#endif

#ifdef BOGUS_INSTANTIATE_3D_SOC
template struct DualFrictionProblem< 3u > ;
template struct PrimalFrictionProblem< 3u > ;
#endif

#ifdef BOGUS_INSTANTIATE_DYNAMIC_SOC
template struct DualFrictionProblem< Eigen::Dynamic > ;
template struct PrimalFrictionProblem< Eigen::Dynamic > ;
#endif

} //namespace bogus
