/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_BLOCK_FWD_HPP
#define BOGUS_BLOCK_FWD_HPP

#include "Block/Traits.hpp"

namespace bogus {

namespace flags
{
	enum {
		NONE = 0,
		COMPRESSED = 0x1,
		COL_MAJOR = 0x2,
		SYMMETRIC = 0x4
	} ;
}

template < typename Derived >
struct BlockObjectBase ;

template < typename Derived >
class BlockMatrixBase ;

template < typename Derived >
class SparseBlockMatrixBase ;

template < typename BlockT, int Flags = flags::NONE >
class SparseBlockMatrix  ;


}

#endif
