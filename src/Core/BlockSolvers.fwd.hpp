/* This file is part of so-bogus, a block-sparse Gauss-Seidel solver          
 * Copyright 2013 Gilles Daviet <gdaviet@gmail.com>                       
 *
 * This Source Code Form is subject to the terms of the Mozilla Public 
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#ifndef BOGUS_GAUSS_SEIDEL_FWD_HPP
#define BOGUS_GAUSS_SEIDEL_FWD_HPP

namespace bogus {

template < typename MatrixType >
struct ProblemTraits ;

template < typename BlockMatrixType >
class GaussSeidel ;

}

#endif