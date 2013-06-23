/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_BLOCKMATRIX_HPP
#define BOGUS_BLOCKMATRIX_HPP

#include "../Block.fwd.hpp"

namespace bogus
{

//! Base empty class, for technical reasons only
struct Object {} ;

//! Base class for anything block
template < typename Derived >
struct BlockObjectBase : public Object
{

	const Derived& derived() const ;
	Derived& derived() ;

	typedef BlockMatrixTraits< Derived > Traits ;
	typedef typename Traits::ConstTransposeReturnType ConstTransposeReturnType ;
	typedef typename Traits::Index Index ;
	typedef typename Traits::PlainObjectType PlainObjectType ;
	typedef typename Traits::EvalType EvalType ;
	typedef typename Traits::Scalar Scalar ;
	enum { is_transposed = Traits::is_transposed } ;

	//! Returns the total number of rows of the matrix ( expanding blocks )
	Index rows() const { return derived().rows() ; }
	//! Returns the total number of columns of the matrix ( expanding blocks )
	Index cols() const { return derived().cols() ; }

	const PlainObjectType* eval() const { return derived().eval() ; }
};

//! Default specialization of traits for BlockMatrices
/*! Re-specialiazed for derived classes, see e.g. BlockMatrixTraits< SparseBlockMatrix > */
template< typename Derived  >
struct BlockMatrixTraits< BlockObjectBase< Derived > > {

	typedef int Index ;
	typedef unsigned BlockPtr ;
	typedef Derived PlainObjectType ;
	typedef const PlainObjectType* EvalType ;

	typedef Transpose< Derived > ConstTransposeReturnType ;
} ;

//! Base class for dense and sparse block matrices, thought dense don't exist yet
template < typename Derived >
class BlockMatrixBase : public BlockObjectBase< Derived >
{
public:
	typedef typename BlockMatrixTraits< Derived >::BlockType BlockType ;
	typedef typename BlockMatrixTraits< Derived >::Index Index ;

	typedef BlockObjectBase< Derived > Base;
	using Base::derived ;

	BlockMatrixBase() : m_rows(0), m_cols(0)
	{}

	virtual ~BlockMatrixBase()
	{}

	//! Performs a matrix vector multiplication
	/*! \tparam Transpose If true, performs \c res = alpha * \c M' * \c rhs + beta * res,
						  otherwise \c res = alpha * M * \c rhs + beta * res
	  */
	template < bool DoTranspose, typename RhsT, typename ResT >
	void multiply( const RhsT& rhs, ResT& res, typename RhsT::Scalar alpha = 1, typename ResT::Scalar beta = 0 ) const
	{
		derived().template multiply< DoTranspose >( rhs, res, alpha, beta ) ;
	}

	//! Multiplies a given block-row of the matrix with \p rhs, omitting the diagonal block
	/*! I.e. res = [ M( row, 0 ) ... M( row, row-1 ) 0 M( row, row+1 ) ... M( row, colsOfBlocks()-1 ) ] * rhs

		Especially useful inside a Gauss-Seidel algorithm.
		\warning Does not work on sparse, non-symmetric column major matrices
	*/
	template < typename RhsT, typename ResT >
	void splitRowMultiply( const Index row, const RhsT& rhs, ResT& res ) const
	{
		derived().splitRowMultiply( row, rhs, res ) ;
	}

	//! Returns a reference to the content of the digonal block of row \param row
	/*! \warning If this block does not exist, the behavior of this function is undefined */
	const BlockType& diagonal( const Index row ) const
	{
		return derived().diagonal( row );
	}

	//! Returns the total number of rows of the matrix ( expanding blocks )
	Index rows() const { return m_rows ; }
	//! Returns the total number of columns of the matrix ( expanding blocks )
	Index cols() const { return m_cols ; }

	//! Returns the number of rows of a given block row
	Index blockRows( Index row ) const { return derived().blockRows( row ) ; }
	//! Returns the number of columns of a given block columns
	Index blockCols( Index col ) const { return derived().blockCols( col ) ; }

	//! Returns the number of block rows of the matrix
	Index rowsOfBlocks() const { return derived().rowsOfBlocks() ; }
	//! Returns the number of block columns of the matrix
	Index colsOfBlocks() const { return derived().colsOfBlocks() ; }

	//! Access to blocks data
	const typename BlockContainerTraits< BlockType >::Type& blocks() const { return  m_blocks ; }
	//! Access to blocks data as a raw pointer
	const BlockType* data() const { return  &m_blocks[0] ; }
	//! Access to blocks data as a raw pointer
	BlockType* data() { return &m_blocks[0] ; }

	const Derived* eval() const { return &derived() ; }

protected:
	Index m_rows ;
	Index m_cols ;

	typename BlockContainerTraits< BlockType >::Type m_blocks ;
} ;


}

#endif // BLOCKMATRIX_HPP
