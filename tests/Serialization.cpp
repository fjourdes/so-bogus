/*
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
*/

#include <bogus/Core/Block.impl.hpp>
#include <bogus/Core/Block.io.hpp>

#ifdef BOGUS_WITH_BOOST_SERIALIZATION

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#include <gtest/gtest.h>

static std::string temp_file_name()
{
	static const char* s_tmpdir  = std::getenv("TMP")     ;
	if( 0 == s_tmpdir ) s_tmpdir = std::getenv("TEMP")    ;
	if( 0 == s_tmpdir ) s_tmpdir = std::getenv("TMPDIR")  ;
	if( 0 == s_tmpdir ) s_tmpdir = "/tmp" ;

	std::ostringstream filename ;
	filename << s_tmpdir ;
	#ifdef WIN32
		filename << '\\' ;
	#else
		filename << '/' ;
	#endif
	filename << "bogus_serialization_test" ;
	return filename.str() ;
}

TEST( Serialization, Eigen )
{

	Eigen::MatrixXd m (3,3) ;
	m << 1, 2, 3, 4, 5, 6, 7, 8, 9 ;
	Eigen::Matrix3d m33 (3,3) ;
	m33 << 2, 4, 6, 8, 5, 6, 7, 8, 9 ;
	Eigen::VectorXd v (3) ;
	v << 1, 2, 3 ;
	Eigen::RowVectorXd rv (3) ;
	rv << 1, 2, 3 ;

	{
		std::ofstream ofs( temp_file_name().c_str() );
		boost::archive::text_oarchive oa(ofs);
		oa << m << m33 << v << rv;
	}

	Eigen::MatrixXd m_ ;
	Eigen::Matrix3d m33_ ;
	Eigen::VectorXd v_;
	Eigen::RowVectorXd rv_;
	{
		std::ifstream ifs( temp_file_name().c_str() );
		boost::archive::text_iarchive ia(ifs);
		ia >> m_ >> m33_ >> v_ >> rv_ ;
	}

	ASSERT_EQ( m, m_ ) ;
	ASSERT_EQ( m33, m33_ ) ;
	ASSERT_EQ( v, v_ ) ;
	ASSERT_EQ( rv, rv_ ) ;
}

#ifdef BOGUS_WITH_EIGEN_STABLE_SPARSE_API

TEST( Serialization, EigenSparse )
{
	Eigen::SparseMatrix< double > sm ( 3, 3 ) ;
	sm.reserve ( 3 );
	sm.insert(0,0) = 1 ;
	sm.insert(1,1) = 2 ;
	sm.insert(2,2) = 3 ;
	sm.makeCompressed() ;

	{
		std::ofstream ofs(temp_file_name().c_str());
		boost::archive::text_oarchive oa(ofs);
		oa << sm;
	}

	Eigen::SparseMatrix< double > sm_;
	{
		std::ifstream ifs(temp_file_name().c_str());
		boost::archive::text_iarchive ia(ifs);
		ia >> sm_ ;
	}

	ASSERT_EQ( Eigen::Vector3d( 1., 2., 3.), sm_ * Eigen::Vector3d::Ones() ) ;
}
#endif // EIGEN >=3.1

TEST( Serialization, SparseBlockMatrix )
{
	Eigen::VectorXd expected( 6 ), rhs ( 4 ) ;
	rhs << 1, 2, 3, 4 ;
	expected << 7, 7, 7, 6, 6, 6 ;

	bogus::SparseBlockMatrix< Eigen::MatrixXd > sbm ;
	sbm.setRows( 2, 3 ) ;
	sbm.setCols( 2, 2 ) ;

	sbm.insertBackAndResize( 0, 1 ).setOnes() ;
	sbm.insertBackAndResize( 1, 0 ).setConstant(2) ;
	sbm.finalize() ;

	bogus::SparseBlockMatrix< Eigen::MatrixXd, bogus::flags::UNCOMPRESSED > sbmc( sbm ) ;

	{
		std::ofstream ofs(temp_file_name().c_str());
		boost::archive::text_oarchive oa(ofs);
		oa << sbm << sbmc ;
	}

	bogus::SparseBlockMatrix< Eigen::MatrixXd > sbm_ ;
	bogus::SparseBlockMatrix< Eigen::MatrixXd, bogus::flags::UNCOMPRESSED > sbmc_ ;

	{
		std::ifstream ifs(temp_file_name().c_str());
		boost::archive::text_iarchive ia(ifs);
		ia >> sbm_ >> sbmc_ ;
	}

	EXPECT_EQ( expected, sbm_*rhs );
	EXPECT_EQ( expected, sbmc_*rhs );
}

TEST( Serialization, Vectors )
{
	bogus::SparseBlockMatrix< Eigen::Vector3d > sbm ;
	sbm.setRows(1) ;
	sbm.setCols(1) ;
	sbm.insertBack(0,0) = Eigen::Vector3d(1.,2.,3.) ;
	sbm.finalize();
	sbm.cacheTranspose();

	bogus::SparseBlockMatrix< Eigen::RowVector3d > sbmc ;
	sbmc.setRows(1) ;
	sbmc.setCols(1) ;
	sbmc.insertBack(0,0) = Eigen::RowVector3d(1.,2.,3.) ;
	sbmc.finalize();
	sbmc.cacheTranspose();


	{
		std::ofstream ofs(temp_file_name().c_str());
		boost::archive::text_oarchive oa(ofs);
		oa << sbm << sbmc ;
	}

	bogus::SparseBlockMatrix< Eigen::Vector3d > sbm_ ;
	bogus::SparseBlockMatrix< Eigen::RowVector3d > sbmc_ ;

	{
		std::ifstream ifs(temp_file_name().c_str());
		boost::archive::text_iarchive ia(ifs);
		ia >> sbm_ >> sbmc_ ;
	}

	EXPECT_EQ( sbm.block(0), sbm_.block(0) );
	EXPECT_EQ( sbm.transposeBlocks()[0], sbm_.transposeBlocks()[0] );

	EXPECT_EQ( sbmc.block(0), sbmc_.block(0) );
	EXPECT_EQ( sbmc.transposeBlocks()[0], sbmc_.transposeBlocks()[0] );

}

TEST( Serialization, CleanUp )
{
	std::remove( temp_file_name().c_str() ) ;
}

#endif // BOGUS_BOOST_SERIALIZATION
