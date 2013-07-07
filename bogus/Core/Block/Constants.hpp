/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver          
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>                       
 *
 * This Source Code Form is subject to the terms of the Mozilla Public 
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_BLOCK_CONSTANTS_HPP
#define BOGUS_BLOCK_CONSTANTS_HPP

#include <cstddef>

#if !( defined( _OPENMP ) || defined( BOGUS_DONT_PARALLELIZE ) )
#define BOGUS_DONT_PARALLELIZE
#endif

#ifndef BOGUS_DEFAULT_INDEX_TYPE
#define BOGUS_DEFAULT_INDEX_TYPE int
#endif

#ifndef BOGUS_DEFAULT_BLOCK_PTR_TYPE
#define BOGUS_DEFAULT_BLOCK_PTR_TYPE unsigned
#endif

namespace bogus
{

//! Flags for compile-time tuning of the behavior of objects such as SparseBlockMatrix
/*! Any combination if those is possible, using the 'binary or' ('|') operator.
 */
namespace flags
{
    enum {
        //! Default value: the matrix will use an uncompressed index, will be row-major, and not symmetric
        NONE = 0,
        //! Use a compressed index
        //! This adds some restrictions on the order in which elements can be inserted,
        //! but can be more efficient and allow interoperability with other formats
        //! such as MKL's BSR
        COMPRESSED = 0x1,
        //! Store and index blocks in a column major way
        COL_MAJOR = 0x2,
        //! Store only half the matrix, or rather the triangular part which verifies \c inner \c <= \c outer ,
        /*! ( \c outer being the row and \c inner the column for row-major matrices )
          * Linear algebra operations, such as matrix vector and matrix matrix multiplication, will work
          * as if the matrix was fully populated, but at a lower memory access cost
          */
        SYMMETRIC = 0x4
    } ;
}
// Reduce verbosity of public API
using namespace flags ;

namespace internal
{
    //! Indicates a value that is not known at compile time
    enum { DYNAMIC = -1 } ;
}

}

#endif
