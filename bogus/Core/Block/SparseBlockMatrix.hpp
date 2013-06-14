/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_SPARSEBLOCKMATRIX_HH
#define BOGUS_SPARSEBLOCKMATRIX_HH

#include "BlockMatrix.hpp"
#include "SparseBlockIndex.hpp"
#include "CompressedSparseBlockIndex.hpp"
#include "Expressions.hpp"

namespace bogus
{

template < typename BlockT, int Flags > class SparseBlockMatrix  ;
template < bool Symmetric > struct SparseBlockMatrixFinalizer ;
template < typename Derived, bool Major > struct SparseBlockIndexGetter ;

//! Base class for SparseBlockMatrix
/*! Most of the useful functions are defined and implemented here, but instantiation
  should be done throught derived classes such as SparseBlockMatrix */
template < typename Derived >
class SparseBlockMatrixBase : public BlockMatrixBase< Derived >
{

public:
	typedef BlockMatrixBase< Derived > Base ;
	typedef BlockMatrixTraits< Derived > Traits ;

	typedef typename Traits::Index Index ;
	typedef typename Traits::SparseIndexType SparseIndexType ;
	typedef typename Traits::RowIndexType RowIndexType ;
	typedef typename Traits::ColIndexType ColIndexType ;
	typedef typename Traits::UncompressedIndexType UncompressedIndexType ;

	typedef typename Traits::BlockType BlockType ;
	typedef typename Traits::BlockPtr BlockPtr ;
	typedef typename Traits::Scalar Scalar ;

	//! Return value of blockPtr( Index, Index ) for non-existing block
	static const BlockPtr InvalidBlockPtr ;

	typedef typename Base::ConstTransposeReturnType  ConstTransposeReturnType ;

	using Base::rows ;
	using Base::cols ;
	using Base::blocks ;
	using Base::derived ;

	//! \name Setting and accessing the matrix structure
	///@{

	//! Defines the row structure of the matrix
	/*! \param nBlocks the number of rows of blocks
		\param rowsPerBlock array containing the number of row of each block
	*/
	void setRows( const Index nBlocks, const unsigned* rowsPerBlock ) ;
	//! Same, using a std::vector
	void setRows( const std::vector< unsigned > &rowsPerBlock )
	{
		setRows( rowsPerBlock.size(), &rowsPerBlock[0] ) ;
	}
	//! Same, setting each block to have exactly \p rowsPerBlock
	void setRows( const Index nBlocks, const Index rowsPerBlock )
	{
		setRows( std::vector< unsigned >( nBlocks, rowsPerBlock ) ) ;
	}
	//! Same, deducing the (constant) number of rows per block from the BlockType
	void setRows( const Index nBlocks )
	{
		setRows( nBlocks, BlockType::RowsAtCompileTime ) ;
	}

	//! Defines the column structure of the matrix
	/*! \param nBlocks the number of columns of blocks
		\param colsPerBlock array containing the number of columns of each block
	*/
	void setCols( const Index nBlocks, const unsigned* colsPerBlock ) ;
	//! Same, using a std::vector
	void setCols( const std::vector< unsigned > &colsPerBlock )
	{
		setCols( colsPerBlock.size(), &colsPerBlock[0] ) ;
	}
	//! Same, setting each block to have exactly \p rowsPerBlock
	void setCols( const Index nBlocks, const Index colsPerBlock )
	{
		setCols( std::vector< unsigned >( nBlocks, colsPerBlock ) ) ;
	}
	//! Same, deducing the (constant) number of rows per block from the BlockType
	void setCols( const Index nBlocks )
	{
		setCols( nBlocks, BlockType::ColsAtCompileTime ) ;
	}

	Index rowsOfBlocks() const { return rowOffsets().size() - 1 ; }
	Index colsOfBlocks() const { return colOffsets().size() - 1 ; }

	Index blockRows( Index row ) const { return rowOffsets()[ row + 1 ] - rowOffsets()[ row ] ; }
	Index blockCols( Index col ) const { return colOffsets()[ col + 1 ] - colOffsets()[ col ] ; }

	///@}

	//! \name Inserting and accessing blocks
	///@{

	//! Reserve enouch memory to accomodate \p nBlocks
	void reserve( std::size_t nBlocks )
	{
		m_blocks.reserve( nBlocks ) ;
	}

	//! Inserts a block in the matrix, and returns a reference to it
	/*! \warning If the matrix is Compressed, the insertion order should be such that the pair
	  ( outerIndex, innerIndex ) is always strictly increasing.
	  That is, if the matrix is row-major, the insertion should be done one row at a time,
	  and for each row from the left-most column to the right most.

	  For non-compressed matrices, no such limitation apply, though out of order insertion might lead
	  to bad cache performance.
	  */
	BlockType& insertBack( Index row, Index col )
	{
		if( Traits::is_col_major )
			return insertBackOuterInner( col, row ) ;
		else
			return insertBackOuterInner( row, col ) ;
	}
	//! Inserts a block and immediately resize it according to the dimensions given to setRows() and setCols()
	BlockType& insertBackAndResize( Index row, Index col )
	{
		BlockType& block = insertBack( row, col ) ;
		block.resize( blockRows( row ), blockCols( col ) ) ;
		return block ;
	}
	//! Inert a block, specifying directily the outer and inner indices instead of row and column
	BlockType& insertBackOuterInner( Index outer, Index inner ) ;

	//! Finalizes the matrix.
	//! \warning Should always be called after all blocks have been inserted, or bad stuff may happen
	void finalize() ;

	//! Clears the matrix
	void clear() ;
	//! \sa clear()
	void setZero() { clear() ; }

	//! Removes all blocks for which \c is_zero( \c block, \c precision ) is \c true
	/*! This function compacts the blocks and rebuild the index, which can be slow */
	Derived& prune( const Scalar precision = 0 ) ;

	//! Returns the number of blocks of the matrices
	/*! \warning This may differ from blocks().size() */
	std::size_t nBlocks() const { return m_nBlocks ; }

	const BlockType& block( BlockPtr ptr ) const
	{
		return m_blocks[ ptr ] ;
	}

	BlockType& block( BlockPtr ptr )
	{
		return m_blocks[ ptr ] ;
	}

	BlockPtr blockPtr( Index row, Index col ) const ;

	//! \warning block has to exist
	BlockType& diagonal( const Index row ) ;
	//! \warning block has to exist
	const BlockType& diagonal( const Index row ) const ;

	//! \warning block has to exist
	BlockType& block( Index row, Index col )
	{ return block( blockPtr( row, col ) ) ; }
	//! \warning block has to exist
	const BlockType& block( Index row, Index col ) const
	{ return block( blockPtr( row, col ) ) ; }

	///@}

	//! \name Accessing and manipulating indexes
	///@{

	//! Computes the minor index of the matrix.
	/*! That is, the column major index for row-major matrices, and vice versa.
		This may speed up some operations, such as matrix/matrix multiplication under
		some circumstances.
	*/
	bool computeMinorIndex() ;

	//! Computes and caches the tranpose of the matrix.
	/*! This will speed up some operations; especially splitRowMultiply() on symmetric matrices */
	void cacheTranspose() ;
	//! Returns whether the transpose has been cached
	bool transposeCached() const { return m_transposeIndex.valid ; }

	const SparseIndexType& majorIndex() const
	{
		return m_majorIndex ;
	}
	const UncompressedIndexType& minorIndex() const
	{
		return m_minorIndex ;
	}
	const SparseIndexType& transposeIndex() const
	{
		return m_transposeIndex ;
	}
	const ColIndexType& colMajorIndex() const ;
	const RowIndexType& rowMajorIndex() const ;

	//@}

	//! \name Assignment and cloning operations
	///@{

	template < bool Transpose, typename OtherDerived >
	Derived& assign ( const SparseBlockMatrixBase< OtherDerived > &source, const Scalar scale = 1 ) ;

	template < typename OtherDerived >
	Derived& operator= ( const SparseBlockMatrixBase< OtherDerived > &source )
	{ return assign< false >( source ) ; }

	template < typename OtherDerived >
	Derived& operator= ( const Transpose< SparseBlockMatrixBase< OtherDerived > > &source )
	{ return assign< true >( source.matrix ) ; }

	template < typename LhsT, typename RhsT >
	Derived& operator= ( const Product< LhsT, RhsT > &prod ) ;

	//! Clones the dimensions ( number of rows/cols blocks and rows/cols per block ) of \p source
	template< typename OtherDerived >
	void cloneDimensions( const BlockMatrixBase< OtherDerived > &source ) ;

	//! Clones the dimensions and the indexes of \p source
	template < typename BlockT2 >
	void cloneStructure( const SparseBlockMatrix< BlockT2, Traits::flags > &source ) ;

	//@}

	//! \name Linear algebra
	//@{

	ConstTransposeReturnType transpose() const { return Transpose< SparseBlockMatrixBase< Derived > >( *this ) ; }

	template < bool Transpose, typename RhsT, typename ResT >
	void multiply( const RhsT& rhs, ResT& res, typename RhsT::Scalar alpha = 1, typename ResT::Scalar beta = 0 ) const ;

	template < typename RhsT, typename ResT >
	void splitRowMultiply( const Index row, const RhsT& rhs, ResT& res ) const ;

	template < bool ColWise, typename LhsT, typename RhsT >
	void setFromProduct( const Product< LhsT, RhsT > &prod ) ;

	//! Performs *this *= alpha
	Derived& scale( Scalar alpha ) ;

	//! Performs *this += alpha * rhs (SAXPY)
	template < typename RhsT >
	void scaleAdd( const BlockObjectBase< RhsT > &rhs, Scalar alpha ) ;


	Derived& operator *= ( Scalar alpha ) { return scale( alpha ) ; }
	Derived& operator /= ( Scalar alpha ) { return scale( 1./alpha ) ; }

	//@}

	//! \name I/O
	//@{

#ifdef BOGUS_WITH_BOOST_SERIALIZATION
	template < typename Archive >
	void serialize( Archive & ar, const unsigned int file_version ) ;
#endif

	//@}

	//! \name Unsafe API
	//@{

	//! Direct access to major index.
	/*! Could be used in conjunction with prealloc() to devise a custom way of building the index.
		Dragons, etc. */
	SparseIndexType& majorIndex() { return m_majorIndex ; }

	//! Resizes \c m_blocks and set \c m_nBlocks to \p nBlocks
	void prealloc( std::size_t nBlocks ) ;

	//@}

protected:

	using Base::m_cols ;
	using Base::m_rows ;
	using Base::m_blocks ;

	typedef SparseBlockMatrixFinalizer< Traits::is_symmetric > Finalizer ;
	friend struct SparseBlockIndexGetter< Derived, true > ;
	friend struct SparseBlockIndexGetter< Derived, false > ;

	SparseBlockMatrixBase() ;

	//! Pushes a block at the back of \c m_blocks, and increments \c m_nBlocks
	void allocateBlock( BlockPtr &ptr ) ;

	void computeMinorIndex( UncompressedIndexType &cmIndex) const ;

	const UncompressedIndexType& getOrComputeMinorIndex( UncompressedIndexType &tempIndex) const ;

	Index rowOffset( Index row ) const { return rowOffsets()[row] ; }
	Index colOffset( Index col ) const { return colOffsets()[col] ; }
	const std::vector< Index >& rowOffsets() const { return colMajorIndex().innerOffsets ; }
	const std::vector< Index >& colOffsets() const { return rowMajorIndex().innerOffsets ; }

	ColIndexType& colMajorIndex() ;
	RowIndexType& rowMajorIndex() ;

	template < typename VecT >
	typename VecT::SegmentReturnType rowSegment( VecT& v, Index rowBlockIdx ) const
	{
		return v.segment( rowOffset( rowBlockIdx ), blockRows( rowBlockIdx ) ) ;
	}

	template < typename VecT >
	typename VecT::ConstSegmentReturnType rowSegment( const VecT& v, Index rowBlockIdx ) const
	{
		return v.segment( rowOffset( rowBlockIdx ), blockRows( rowBlockIdx ) ) ;
	}

	template < typename VecT >
	typename VecT::SegmentReturnType colSegment( VecT& v, Index colBlockIdx ) const
	{
		return v.segment( colOffset( colBlockIdx ), blockCols( colBlockIdx ) ) ;
	}

	template < typename VecT >
	typename VecT::ConstSegmentReturnType colSegment( const VecT& v, Index colBlockIdx ) const
	{
		return v.segment( colOffset( colBlockIdx ), blockCols( colBlockIdx ) ) ;
	}

	template< typename IndexT >
	void setInnerOffets( IndexT& index, const Index nBlocks, const unsigned* blockSizes ) const ;

	template < bool ColWise, typename LhsIndex, typename RhsIndex, typename LhsBlock, typename RhsBlock, typename LhsGetter, typename RhsGetter  >
	void setFromProduct( const LhsIndex &lhsIdx, const RhsIndex &rhsIdx,
						 const LhsBlock  *lhsData,
						 const RhsBlock  *rhsData,
						 const LhsGetter &lhsGetter,
						 const RhsGetter &rhsGetter
						  ) ;

	//! Number of blocks on the matrix. Can be different of blocks().size(), for example when the transpose is cached.
	std::size_t m_nBlocks ;

	SparseIndexType m_majorIndex ;
	// Minor index is always uncompressed, as the blocks cannot be contiguous
	// For a symmetric matrix, do not store diagonal block in the minor and transpose index
	UncompressedIndexType m_minorIndex ;
	SparseIndexType m_transposeIndex ;
} ;

//! Specialization of BlockMatrixTraits for SparseBlockMatrix
template < typename BlockT, int Flags >
struct BlockMatrixTraits< SparseBlockMatrix< BlockT, Flags > > : public BlockMatrixTraits< BlockObjectBase< SparseBlockMatrix< BlockT, Flags > > >
{
	typedef BlockMatrixTraits< BlockObjectBase< SparseBlockMatrix< BlockT, Flags > > > BaseTraits ;
	typedef typename BaseTraits::Index Index ;
	typedef typename BaseTraits::BlockPtr BlockPtr ;

	typedef BlockT BlockType ;
	typedef typename BlockTraits< BlockT >::Scalar Scalar ;

	enum {
		is_compressed  = Flags & flags::COMPRESSED,
		is_symmetric   = Flags & flags::SYMMETRIC,
		is_col_major   = Flags & flags::COL_MAJOR
	} ;
	enum {
		flags         = Flags
	} ;

	typedef SparseBlockIndex< is_compressed, Index, BlockPtr > SparseIndexType ;

	typedef SparseBlockIndex< false, Index, BlockPtr  > UncompressedIndexType ;
	typedef SparseBlockIndex< is_compressed && !is_col_major, Index, BlockPtr > RowIndexType ;
	typedef SparseBlockIndex< is_compressed && is_col_major , Index, BlockPtr > ColIndexType ;

	typedef Transpose< SparseBlockMatrixBase< SparseBlockMatrix< BlockT, Flags > > > ConstTransposeReturnType ;
} ;

//! Sparse Block Matrix
/*!
  \tparam BlockT the type of the blocks of the matrix. Can be scalar, Eigen dense of sparse matrices,
  or basically anything provided a few functions are specialized
  \tparam Flags a combination of the values defined in \ref bogus::flags
  */
template < typename BlockT, int Flags >
class SparseBlockMatrix : public  SparseBlockMatrixBase< SparseBlockMatrix< BlockT, Flags > >
{
	typedef SparseBlockMatrixBase< SparseBlockMatrix< BlockT, Flags > > Base ;
public:
	SparseBlockMatrix() : Base() {}

	template < typename RhsT >
	SparseBlockMatrix( const BlockObjectBase< RhsT >& rhs ) : Base()
	{
		Base::operator= ( rhs.derived() ) ;
	}

	template < typename RhsT >
	SparseBlockMatrix& operator=( const BlockObjectBase< RhsT >& rhs )
	{
		return ( Base::operator= ( rhs.derived() ) ).derived() ;
	}

	template < typename RhsT >
	SparseBlockMatrix( const Transpose< SparseBlockMatrixBase< RhsT > > & rhs ) : Base()
	{
		Base::operator= ( rhs ) ;
	}

	template < typename RhsT >
	SparseBlockMatrix& operator=( const Transpose< SparseBlockMatrixBase< RhsT > > & rhs )
	{
		return ( Base::operator= ( rhs ) ).derived() ;
	}
} ;

}

#endif // SPARSEBLOCKMATRIX_HH
