/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_BLOCK_ACCESS_HPP
#define BOGUS_BLOCK_ACCESS_HPP

#include "Constants.hpp"

namespace bogus {

//! Specialization of transpose_block() for self-adjoint types
template < typename SelfTransposeT >
inline const typename SelfTransposeTraits< SelfTransposeT >::ReturnType& transpose_block( const SelfTransposeT &block  ) { return  block ; }

//! Utility struct for expressing a compile-time conditional transpose of a block
template < bool DoTranspose >
struct BlockGetter {
	template < typename BlockT >
	inline static const BlockT& get( const BlockT& src, bool = false )
	{ return src ; }
} ;
template < >
struct BlockGetter< true > {
	//SFINAE, as we can't use decltype()

	template < typename BlockT >
	inline static typename BlockT::ConstTransposeReturnType get( const BlockT& src, bool = false )
	{ return src.transpose() ; }

	template < typename BlockT >
	inline static typename BlockTransposeTraits< typename BlockT::Base >::ReturnType get( const BlockT& src, bool = false )
	{ return transpose_block( src ) ; }

	template < typename BlockT >
	inline static typename BlockTransposeTraits< BlockT >::ReturnType get( const BlockT& src, bool = false )
	{ return transpose_block( src ) ; }

	template < typename BlockT >
	inline static typename SelfTransposeTraits< BlockT >::ReturnType get( const BlockT& src, bool = false )
	{ return src ; }
} ;

template < bool RuntimeCheck, bool DoTranspose >
struct BlockTransposeOption : public BlockGetter< DoTranspose >{
} ;

template < bool IgnoredDoTranspose >
struct BlockTransposeOption< true, IgnoredDoTranspose > {
	template < typename BlockT >
	inline static BlockT get( const BlockT& src, bool doTranspose = false )
	{ return doTranspose ? BlockT( transpose_block( src ) ) : src ; }
} ;

template< typename BlockT, bool Transpose_ = false >
struct BlockDims
{
	typedef BlockTraits< BlockT > Traits ;
	enum { Rows = Traits::RowsAtCompileTime,
		   Cols = Traits::ColsAtCompileTime } ;
} ;
template< typename BlockT >
struct BlockDims< BlockT, true >

{
	typedef BlockTraits< BlockT > Traits ;
	enum { Rows = Traits::ColsAtCompileTime,
		   Cols = Traits::RowsAtCompileTime } ;
} ;


//! Access to segment of a vector corresponding to a given block-row
template < int DimensionAtCompileTime, typename VectorType, typename Index >
struct Segmenter
{
	enum { dimension = DimensionAtCompileTime } ;

	typedef typename VectorType::template NRowsBlockXpr< dimension >::Type
	ReturnType ;
	typedef typename VectorType::template ConstNRowsBlockXpr< dimension >::Type
	ConstReturnType ;

	Segmenter( VectorType &vec, const Index* ) : m_vec( vec ) {}

	inline ReturnType operator[]( const Index inner )
	{
		return m_vec.template middleRows< dimension >( dimension*inner ) ;
	}

	inline ConstReturnType operator[]( const Index inner ) const
	{
		return m_vec.template middleRows< dimension >( dimension*inner ) ;
	}

private:
	VectorType &		 m_vec ;
} ;

template < typename VectorType, typename Index >
struct Segmenter< internal::DYNAMIC, VectorType, Index >
{
	typedef typename VectorType::RowsBlockXpr			ReturnType ;
	typedef typename VectorType::ConstRowsBlockXpr		ConstReturnType ;

	Segmenter( VectorType &vec, const Index* offsets ) : m_vec( vec ), m_offsets( offsets ) { }

	inline ReturnType operator[]( const Index inner )
	{
		return m_vec.middleRows( m_offsets[ inner ], m_offsets[ inner + 1 ] - m_offsets[ inner ] ) ;
	}

	inline ConstReturnType operator[]( const Index inner ) const
	{
		return m_vec.middleRows( m_offsets[ inner ], m_offsets[ inner + 1 ] - m_offsets[ inner ] ) ;
	}


private:
	VectorType &		 m_vec ;
	const Index* m_offsets ;
} ;

}

#endif
