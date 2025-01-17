
/*
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
*/

#include <gtest/gtest.h>

#include <bogus/Core/Utils/CppTools.hpp>
#include <bogus/Core/Utils/Polynomial.hpp>
#include <bogus/Core/Utils/Polynomial.impl.hpp>

#include <bogus/Core/Eigen/EigenProblemTraits.hpp>
#include <bogus/Core/Eigen/EigenLinearSolvers.hpp>
#include <bogus/Extra/SOC/LocalSOCSolver.impl.hpp>

TEST( Polynomial, Quadratic )
{
	double c[2] = {-1, 0} ;
	double x[2] ;

	unsigned nRoots = bogus::polynomial::getRealRoots( c, x ) ;
	EXPECT_EQ( 2u, nRoots ) ;
	nRoots = bogus::polynomial::getRealRoots( c, x, bogus::polynomial::StrictlyPositiveRoots ) ;
	EXPECT_EQ( 1u, nRoots ) ;
	EXPECT_DOUBLE_EQ(  1., x[0] ) ;
	nRoots = bogus::polynomial::getRealRoots( c, x, bogus::polynomial::StrictlyNegativeRoots ) ;
	EXPECT_EQ( 1u, nRoots ) ;
	EXPECT_DOUBLE_EQ(  -1., x[0] ) ;

	c[0] = 1 ;
	nRoots = bogus::polynomial::getRealRoots( c, x ) ;
	EXPECT_EQ( 0u, nRoots ) ;
	c[1] = -2 ;
	nRoots = bogus::polynomial::getRealRoots( c, x ) ;
	EXPECT_EQ( 2u, nRoots ) ;
	EXPECT_DOUBLE_EQ(  1., x[0] ) ;
	EXPECT_DOUBLE_EQ(  1., x[1] ) ;
}

TEST( Polynomial, SOQPQuartic )
{

	Eigen::Matrix3d W ;
	W <<  0.01344, -9.421e-07, 0.001486,
		  -9.421e-07, 0.1061, 0.0001733,
		  0.001486, 0.0001733, 0.001442 ;
	Eigen::Vector3d b ;
	b << -0.1458, -0.2484, -0.1515 ;

	Eigen::Vector3d r ;
	Eigen::Vector3d u = Eigen::Vector3d::Zero() ;
	double mu = 0.6 ;

	double res = bogus::LocalSOCSolver< 3, double, false, bogus::local_soc_solver::PureEnumerative >
			::solve( W, b, r, mu, 1.e-12, 1 ) ;

	u = W*r + b ;

	 ASSERT_GT( 1.e-24, res ) ;
	 ASSERT_TRUE( r[0] > 0 ) ;
	 ASSERT_TRUE( u[0] > 0 ) ;
	 ASSERT_FLOAT_EQ( float(u[0]), float(u.segment<2>(1) .norm() * mu) ) ;
	 ASSERT_TRUE( u.dot(r) < 1.e-12 ) ;


	 // From fclib/fix/NESpheres_30_1.hdf5

	 mu = 0.8 ;

	 W << 0.4,   0,   0,
	        0, 0.9,   0,
	        0,   0, 0.9;
	 b <<  0, 0, 0.542629 ;

	 res = bogus::LocalSOCSolver< 3, double, false, bogus::local_soc_solver::PureEnumerative >
			::solve( W, b, r, mu, 1.e-12, 1 ) ;
	 u = W*r + b ;
	 ASSERT_GT( 1.e-24, res ) ;
	 ASSERT_TRUE( r[0] > 0 ) ;
	 ASSERT_TRUE( u[0] > 0 ) ;
	 ASSERT_FLOAT_EQ( float(u[0]), float(u.segment<2>(1) .norm() * mu) ) ;
	 ASSERT_TRUE( u.dot(r) < 1.e-12 ) ;

	 // r_newton : 0.444777         0 -0.355821
	 // std::cout << r.transpose() << std::endl ;

	 W << 0.4,   0,   0,
	        0, 1.4,   0,
	        0,   0, 1.4;
	 b << -0.128762, 3.21747e-16,    14.5114 ;

	 res = bogus::LocalSOCSolver< 3, double, false, bogus::local_soc_solver::PureEnumerative >
	         ::solve( W, b, r, mu, 1.e-12, 1 ) ;
	 u = W*r + b ;
	 ASSERT_GT( 1.e-24, res ) ;
	 ASSERT_TRUE( r[0] > 0 ) ;
	 ASSERT_TRUE( u[0] > 0 ) ;
	 ASSERT_FLOAT_EQ( float(u[0]), float(u.segment<2>(1) .norm() * mu) ) ;
	 ASSERT_TRUE( u.dot(r) < 1.e-12 ) ;

	 // r_newton : 9.05699 -1.60649e-16     -7.24559
	 // std::cout << r.transpose() << std::endl ;
}

