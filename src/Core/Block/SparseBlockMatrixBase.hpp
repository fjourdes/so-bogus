/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_SPARSEBLOCKMATRIX_BASE_HPP
#define BOGUS_SPARSEBLOCKMATRIX_BASE_HPP

#include "BlockMatrixBase.hpp"
#include "Expressions.hpp"

#include "SparseBlockIndex.hpp"
#include "CompressedSparseBlockIndex.hpp"

#include "../Utils/CppTools.hpp"

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
    // Convenient typedefs	and using directives

    typedef BlockMatrixBase< Derived > Base ;
    typedef BlockMatrixTraits< Derived > Traits ;

    typedef typename Traits::BlockPtr               BlockPtr ;
    typedef typename Base::Index                    Index ;

    typedef typename Traits::MajorIndexType         MajorIndexType ;

    typedef SparseBlockIndex< false, Index, BlockPtr > UncompressedIndexType ;
    typedef SparseBlockIndex<  true, Index, BlockPtr > CompressedIndexType ;

    // Minor index is always uncompressed, as the blocks cannot be contiguous
    // For a symmetric matrix, it does not store diagonal block in the minor and transpose index
    typedef UncompressedIndexType MinorIndexType ;
    // Transpose index is compressed for perf, as we always create it in a compressed-compatible way
    typedef CompressedIndexType TransposeIndexType ;

    typedef typename TypeSwapIf< Traits::is_col_major, MajorIndexType, MinorIndexType >::First  RowIndexType ;
    typedef typename TypeSwapIf< Traits::is_col_major, MajorIndexType, MinorIndexType >::Second ColIndexType ;

    typedef typename Base::BlockType                BlockType ;
    typedef typename Base::Scalar                   Scalar ;

    // Canonical type for a mutable matrix with different block type
    template < typename OtherBlockType >
    struct MutableImpl
    {
        typedef SparseBlockMatrix< OtherBlockType, Traits::flags & ~flags::UNCOMPRESSED > Type ;
    } ;

    typedef typename Base::ConstTransposeReturnType ConstTransposeReturnType ;

    using Base::rows ;
    using Base::cols ;
    using Base::blocks ;
    using Base::derived ;

    //! Return value of blockPtr( Index, Index ) for non-existing block
    static const BlockPtr InvalidBlockPtr ;


    // Public API

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

    Index rowsOfBlocks() const { return rowMajorIndex().outerSize() ; }
    Index colsOfBlocks() const { return colMajorIndex().outerSize() ; }

    Index blockRows( Index row ) const { return rowOffsets()[ row + 1 ] - rowOffsets()[ row ] ; }
    Index blockCols( Index col ) const { return colOffsets()[ col + 1 ] - colOffsets()[ col ] ; }

    ///@}

    //! \name Inserting and accessing blocks
    ///@{

    //! Reserve enouch memory to accomodate \p nBlocks
    void reserve( std::size_t nBlocks )
    {
        m_blocks.reserve( nBlocks ) ;
        m_majorIndex.reserve( nBlocks ) ;
    }

    //! Inserts a block in the matrix, and returns a reference to it
    /*! \warning If the matrix is Compressed, the insertion order should be such that the pair
      ( outerIndex, innerIndex ) is always strictly increasing.
      That is, if the matrix is row-major, the insertion should be done one row at a time,
      and for each row from the left-most column to the right most.

      For non-compressed matrices, this limitation does not apply, though out of order insertion might lead
      to bad cache performance, and a std::sort will be performed on each inner vector so we
      can keep efficient block( row, col ) look-up functions. The name \a insertBack  is kept
      to make the caller think twice about out-of-order insertion
      */
    BlockType& insertBack( Index row, Index col )
    {
        if( Traits::is_col_major )
            return insertBackOuterInner( col, row ) ;
        else
            return insertBackOuterInner( row, col ) ;
    }

    //! Convenience method that insertBack() a block and immediately resize it according to the dimensions given to setRows() and setCols()
    BlockType& insertBackAndResize( Index row, Index col )
    {
        BlockType& block = insertBack( row, col ) ;
        block.resize( blockRows( row ), blockCols( col ) ) ;
        return block ;
    }

    //! Insert a block, specifying directily the outer and inner indices instead of row and column
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

    //! Returns whether the matrix is empty
    bool empty() const { return 0 == nBlocks() ; }

    const BlockType& block( BlockPtr ptr ) const
    {
        return m_blocks[ ptr ] ;
    }

    BlockType& block( BlockPtr ptr )
    {
        return m_blocks[ ptr ] ;
    }

    //! Return a BlockPtr to the block a (row, col) or InvalidBlockPtr if it does not exist
    BlockPtr blockPtr( Index row, Index col ) const ;
    //! Return a BlockPtr to the block a (row, row) or InvalidBlockPtr if it does not exist
    BlockPtr diagonalBlockPtr( Index row  ) const ;

    //! \warning block has to exist
    BlockType& diagonal( const Index row )
    { return block( diagonalBlockPtr( row ) ) ; }
    //! \warning block has to exist
    const BlockType& diagonal( const Index row ) const
    { return block( diagonalBlockPtr( row ) ) ; }

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
    /*! This will speed up some operations; especially splitRowMultiply() on symmetric matrices
      \warning Since the transposed blocks will be stored at the end of the blocks() array,
      the blocks should either be square or have dynamic sizes ( e.e they should be assignable )
    */
    void cacheTranspose() ;
    //! Returns whether the transpose has been cached
    bool transposeCached() const { return m_transposeIndex.valid ; }

    const MajorIndexType& majorIndex() const
    {
        return m_majorIndex ;
    }
    const MinorIndexType& minorIndex() const
    {
        return m_minorIndex ;
    }
    const TransposeIndexType& transposeIndex() const
    {
        return m_transposeIndex ;
    }
    const ColIndexType& colMajorIndex() const ;
    const RowIndexType& rowMajorIndex() const ;

    //@}

    //! \name Assignment and cloning operations
    ///@{

    //! Performs ( *this = scale * source ) or ( *this = scale * source.transpose() )
    template < bool Transpose, typename OtherDerived >
    Derived& assign ( const SparseBlockMatrixBase< OtherDerived > &source, const Scalar scale = 1 ) ;

    template < typename OtherDerived >
    Derived& operator= ( const BlockObjectBase< OtherDerived > &source )
    {
        typename OtherDerived::EvalType rhs ( source.eval() ) ;
        return assign< OtherDerived::is_transposed >( *rhs ) ;
    }

    Derived& operator= ( const SparseBlockMatrixBase &source ) ;

    template < typename LhsT, typename RhsT >
    Derived& operator= ( const Product< LhsT, RhsT > &prod ) ;

    template < typename LhsT, typename RhsT >
    Derived& operator= ( const Addition< LhsT, RhsT > &prod ) ;

    template < typename OtherDerived >
    Derived& operator= ( const Scaling< OtherDerived > &scaling )
    {
        typename Scaling< OtherDerived >::Operand::EvalType operand( scaling.operand.object.eval() ) ;
        return assign< Scaling< OtherDerived >::transposeOperand >( *operand, scaling.operand.scaling ) ;
    }

    //! Clones the dimensions ( number of rows/cols blocks and rows/cols per block ) of \p source
    template< typename OtherDerived >
    void cloneDimensions( const BlockMatrixBase< OtherDerived > &source ) ;

    //! Clones the dimensions and the indexes of \p source
    template< typename OtherDerived >	
	void cloneStructure( const SparseBlockMatrixBase< OtherDerived > &source ) ;

    //@}

    //! \name Linear algebra
    //@{

    ConstTransposeReturnType transpose() const { return Transpose< Derived >( derived() ) ; }

    template < bool Transpose, typename RhsT, typename ResT >
    void multiply( const RhsT& rhs, ResT& res, Scalar alpha = 1, Scalar beta = 0 ) const ;

    template < typename RhsT, typename ResT >
    void splitRowMultiply( const Index row, const RhsT& rhs, ResT& res ) const ;

    template < bool ColWise, typename LhsT, typename RhsT >
    void setFromProduct( const Product< LhsT, RhsT > &prod ) ;

    //! Performs *this *= alpha
    Derived& scale( Scalar alpha ) ;

    //! Performs *this += alpha * rhs (SAXPY)
    template < bool Transpose, typename OtherDerived >
    Derived& add( const SparseBlockMatrixBase< OtherDerived > &rhs, Scalar alpha = 1) ;

    //! Coeff-wise multiplication with a scalar
    Derived& operator *= ( Scalar alpha ) { return scale( alpha ) ; }
    //! Coeff-wise division with a scalar
    Derived& operator /= ( Scalar alpha ) { return scale( 1./alpha ) ; }

    //! Unary minus
    Scaling< Derived > operator-() const
    { return Scaling< Derived >( derived(), -1 ) ; }

    //! Adds another SparseBlockMatrixBase to this one
    template < typename OtherDerived >
    Derived& operator+= ( const BlockObjectBase< OtherDerived > &source )
    {
        typename OtherDerived::EvalType rhs ( source.eval() ) ;
        return add< OtherDerived::is_transposed >( *rhs ) ;
    }

    //! Substracts another SparseBlockMatrixBase from this one
    template < typename OtherDerived >
    Derived& operator-= ( const BlockObjectBase< OtherDerived > &source )
    {
        typename OtherDerived::EvalType rhs ( source.eval() ) ;
        return add< OtherDerived::is_transposed >( *rhs, -1 ) ;
    }

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
    MajorIndexType& majorIndex() { return m_majorIndex ; }

    //! Resizes \c m_blocks and set \c m_nBlocks to \p nBlocks
    void prealloc( std::size_t nBlocks ) ;

    //@}

    //! Returns an array containing the first index of each row
    const Index* rowOffsets() const { return colMajorIndex().innerOffsetsData() ; }

    //! Returns an array containing the first index of each column
    const Index* colOffsets() const { return rowMajorIndex().innerOffsetsData() ; }

protected:

    enum {
        is_bsr_compatible =
                Base::has_fixed_size_blocks && Base::has_square_or_dynamic_blocks &&
                Traits::is_compressed && !Traits::is_col_major &&
                BlockTraits< BlockType >::uses_plain_array_storage
    } ;

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

    ColIndexType& colMajorIndex() ;
    RowIndexType& rowMajorIndex() ;

    template< typename IndexT >
    void setInnerOffets( IndexT& index, const Index nBlocks, const unsigned* blockSizes ) const ;

    template < bool ColWise, typename LhsIndex, typename RhsIndex, typename LhsBlock, typename RhsBlock,
               typename LhsGetter, typename RhsGetter >
    void setFromProduct( const LhsIndex &lhsIdx, const RhsIndex &rhsIdx,
                         const LhsBlock  *lhsData,
                         const RhsBlock  *rhsData,
                         const LhsGetter &lhsGetter,
                         const RhsGetter &rhsGetter,
                         Scalar scaling
                         ) ;

    //! Number of blocks on the matrix. Can be different of blocks().size(), for example when the transpose is cached.
    std::size_t m_nBlocks ;

    MajorIndexType m_majorIndex ;

    UncompressedIndexType m_minorIndex ;
    CompressedIndexType   m_transposeIndex ;
} ;

}

#endif // SPARSEBLOCKMATRIX_HH