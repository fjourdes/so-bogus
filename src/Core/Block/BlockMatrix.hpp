/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver          
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>                       
 *
 * This Source Code Form is subject to the terms of the Mozilla Public 
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_BLOCKMATRIX_HPP
#define BOGUS_BLOCKMATRIX_HPP

#include "../Block.fwd.hpp"

#include <vector>

#if !( defined( _OPENMP ) || defined( BOGUS_DONT_PARALLELIZE ) )
#define BOGUS_DONT_PARALLELIZE
#endif

namespace bogus
{

template < typename Derived >
struct BlockObjectBase
{
	typedef Derived PlainObjectType ;

	const Derived& derived() const ;
	Derived& derived() ;
};

template< typename Derived >
struct BlockMatrixTraits< BlockObjectBase< Derived > > {
	typedef unsigned Index ;
	typedef unsigned BlockPtr ;
	typedef Derived PlainObjectType ;
} ;

template < typename Derived >
class BlockMatrixBase : public BlockObjectBase< Derived >
{
public:
	typedef BlockObjectBase< Derived > Base;
	typedef typename BlockMatrixTraits< Derived >::BlockType BlockType ;
	typedef typename BlockMatrixTraits< Derived >::Index Index ;

	using Base::derived ;

	BlockMatrixBase() : m_rows(0), m_cols(0)
	{}

	virtual ~BlockMatrixBase()
	{}

	template < bool Transpose, typename RhsT, typename ResT >
	void multiply( const RhsT& rhs, ResT& res ) const
	{
		derived().template multiply< Transpose >( rhs, res ) ;
	}

	template < typename RhsT, typename ResT >
	void splitRowMultiply( const Index row, const RhsT& rhs, ResT& res ) const
	{
		derived().splitRowMultiply( row, rhs, res ) ;
	}


	const BlockType& diagonal( const Index row ) const
	{
		return derived().diagonal( row );
	}

	Index rows() const { return m_rows ; }
	Index cols() const { return m_cols ; }
	Index rowsOfBlocks() const { return this->derived().rowsOfBlocks() ; }
	Index colsOfBlocks() const { return this->derived().colsOfBlocks() ; }

	const std::vector< BlockType >& blocks() const { return  m_blocks ; }

protected:
	Index m_rows ;
	Index m_cols ;

	std::vector< BlockType > m_blocks ;
} ;


}

#endif // BLOCKMATRIX_HPP
