//@HEADER
// ************************************************************************
// 
//         Claps: A Collection of Domain Decomposition Preconditioners
//                and Solvers
//         Copyright (2006) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
// USA
// Questions? Contact Clark R. Dohrmann (crdohrm@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef PRECONDITIONER_CRD_HPP
#define PRECONDITIONER_CRD_HPP

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "myzero.hpp"

using namespace std;

class preconditioner_crd 
{
public: // functions
  preconditioner_crd() { };
  virtual ~preconditioner_crd() { };
  virtual double norm2(double a[], int n)=0;
  virtual double dotprod(double a[], double b[], int n)=0;
  virtual void sum_vectors(double a[], int n, double a_sum[])=0;
  virtual int initialize_solve(double u[], double r[])=0;
  virtual void apply_preconditioner(const double r[], double z[])=0;
  virtual void A_times_x(double x[] , double Ax[])=0;
private:
protected:
  
};
#endif // PRECONDITIONER_CRD_HPP