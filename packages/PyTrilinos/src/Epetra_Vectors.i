// -*- c++ -*-

// @HEADER
// ***********************************************************************
//
//              PyTrilinos: Python Interface to Trilinos
//                 Copyright (2005) Sandia Corporation
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Bill Spotz (wfspotz@sandia.gov)
//
// ***********************************************************************
// @HEADER

%{
// Epetra includes
#include "Epetra_MultiVector.h"
#include "Epetra_Vector.h"
#include "Epetra_FEVector.h"
#include "Epetra_IntVector.h"

// Local includes
#include "Epetra_NumPyIntVector.h"
#include "Epetra_NumPyMultiVector.h"
#include "Epetra_NumPyVector.h"
#include "Epetra_NumPyFEVector.h"
%}

//////////////
// Typemaps //
//////////////
%epetra_array_typemaps(IntVector  )
%epetra_array_typemaps(MultiVector)
%epetra_array_typemaps(Vector     )
%epetra_array_typemaps(FEVector   )

//////////////////////////////
// Epetra_IntVector support //
//////////////////////////////
%inline
{
  struct IntVector{ };
}
%include "Epetra_IntVector.h"

////////////////////////////////
// Epetra_MultiVector support //
////////////////////////////////
%ignore Epetra_MultiVector::operator()(int) const;
%ignore Epetra_MultiVector::ExtractCopy(double *, int   ) const;
%ignore Epetra_MultiVector::ExtractCopy(double **       ) const;
%ignore Epetra_MultiVector::ExtractView(double **, int *) const;
%ignore Epetra_MultiVector::ExtractView(double ***      ) const;
%ignore Epetra_MultiVector::Dot(const Epetra_MultiVector&,double*) const;
%ignore Epetra_MultiVector::Norm1(double*) const;
%ignore Epetra_MultiVector::Norm2(double*) const;
%ignore Epetra_MultiVector::NormInf(double*) const;
%ignore Epetra_MultiVector::NormWeighted(const Epetra_MultiVector&,double*) const;
%ignore Epetra_MultiVector::MinValue(double*) const;
%ignore Epetra_MultiVector::MaxValue(double*) const;
%ignore Epetra_MultiVector::MeanValue(double*) const;
%ignore Epetra_MultiVector::ResetView(double **);
%ignore Epetra_MultiVector::Pointers() const;
%inline
{
  struct MultiVector{ };
}
%include "Epetra_MultiVector.h"

///////////////////////////
// Epetra_Vector support //
///////////////////////////
%ignore Epetra_Vector::ExtractCopy(double * ) const;
%ignore Epetra_Vector::ExtractView(double **) const;
%ignore Epetra_Vector::ReplaceGlobalValues(int,double*,int*);
%ignore Epetra_Vector::ReplaceGlobalValues(int,int,double*,int*);
%ignore Epetra_Vector::ReplaceMyValues(int,double*,int*);
%ignore Epetra_Vector::ReplaceMyValues(int,int,double*,int*);
%ignore Epetra_Vector::SumIntoGlobalValues(int,double*,int*);
%ignore Epetra_Vector::SumIntoGlobalValues(int,int,double*,int*);
%ignore Epetra_Vector::SumIntoMyValues(int,double*,int*);
%ignore Epetra_Vector::SumIntoMyValues(int,int,double*,int*);
%ignore Epetra_Vector::ResetView(double *);
%inline
{
  struct Vector{ };
}
%include "Epetra_Vector.h"

/////////////////////////////
// Epetra_FEVector support //
/////////////////////////////
%ignore Epetra_FEVector::ReplaceGlobalValues(int,int*,double*);
%ignore Epetra_FEVector::SumIntoGlobalValues(int,int*,double*);
%inline
{
  struct FEVector{ };
}
%include "Epetra_FEVector.h"

///////////////////////////////////
// Epetra_NumPyIntVector support //
///////////////////////////////////
%rename(NumPyIntVector) Epetra_NumPyIntVector;
%epetra_numpy_ctor_exception(Epetra_NumPyIntVector)
%include "Epetra_NumPyIntVector.h"
%pythoncode
%{
class IntVector(UserArray,NumPyIntVector):
    """
    Epetra.IntVector: A class for constructing and using dense integer vectors
    on a parallel computer.

    The Epetra.IntVector class enables the construction and use of integer dense
    vectors in a distributed memory environment. The distribution of the dense
    vector is determined in part by a Epetra.Comm object and a Epetra.Map (or
    Epetra.LocalMap or Epetra.BlockMap).

    Distributed Global vs. Replicated Local Distributed Global Vectors -
    In most instances, a multi-vector will be partitioned across multiple
    memory images associated with multiple processors. In this case, there
    is a unique copy of each element and elements are spread across all
    processors specified by the Epetra.Comm communicator.

    Replicated Local Vectors - Some algorithms use vectors that are too
    small to be distributed across all processors. Replicated local
    vectors handle these types of situation.

    In the python implementation, the IntVector stores an underlying numpy
    array, with which it shares the data buffer.  Also, almost all numpy array
    methods and operators are supported.
    """
    def __init__(self, *args):
        """
        __init__(self, BlockMap map, bool zeroOut=True) -> IntVector
        __init__(self, IntVector source) -> IntVector
        __init__(self, BlockMap map, PyObject array) -> IntVector
        __init__(self, PyObject array) -> IntVector

        Arguments:
            map      - BlockMap describing domain decomposition
            zeroOut  - Flag controlling whether to initialize IntVector to 0
            source   - Source IntVector for copy constructor
            array    - Python sequence that can be converted to a numpy array of
                       integers for initialization
        """
        NumPyIntVector.__init__(self, *args)
        self.__initArray__()
    def __initArray__(self):
        """
        __initArray__(self)
        
        Initialize the underlying numpy array.
        """
        UserArray.__init__(self, self.ExtractView(), dtype="i", copy=False)
    def __str__(self):
        """
        __str__(self)__ -> string
        
        Return a numpy-style string representation of the IntVector.
        """
        return str(self.array)
    def __lt__(self,other):
        """
        __lt__(self, other) -> bool

        Less-than operator (<).
        """
        return numpy.less(self.array,other)
    def __le__(self,other):
        """
        __le__(self, other) -> bool

        Less-than-or-equal operator (<=).
        """
        return numpy.less_equal(self.array,other)
    def __eq__(self,other):
        """
        __eq__(self, other) -> bool

        Equal operator (==).
        """
        return numpy.equal(self.array,other)
    def __ne__(self,other):
        """
        __ne__(self, other) -> bool

        Not-equal operator (!=).
        """
        return numpy.not_equal(self.array,other)
    def __gt__(self,other):
        """
        __gt__(self, other) -> bool

        Greater-than operator (>).
        """
        return numpy.greater(self.array,other)
    def __ge__(self,other):
        """
        __ge__(self, other) -> bool

        Greater-than-or-equal operator (>=).
        """
        return numpy.greater_equal(self.array,other)
    def __getattr__(self, key):
        # This should get called when the IntVector is accessed after not
        # properly being initialized
        if not "array" in self.__dict__:
            self.__initArray__()
        try:
            return self.array.__getattribute__(key)
        except AttributeError:
            return IntVector.__getattribute__(self, key)
    def __setattr__(self, key, value):
        "Handle 'this' properly and protect the 'array' attribute"
        if key == "this":
            NumPyIntVector.__setattr__(self, key, value)
        else:
            if key == "array":
                if key in self.__dict__:
                    raise AttributeError, \
                          "Cannot change Epetra.IntVector array attribute"
            UserArray.__setattr__(self, key, value)
_Epetra.NumPyIntVector_swigregister(IntVector)
%}

/////////////////////////////////////
// Epetra_NumPyMultiVector support //
/////////////////////////////////////
%rename(NumPyMultiVector) Epetra_NumPyMultiVector;
%epetra_numpy_ctor_exception(Epetra_NumPyMultiVector)
%include "Epetra_NumPyMultiVector.h"
%pythoncode
%{
class MultiVector(UserArray,NumPyMultiVector):
    """
    Epetra.MultiVector: A class for constructing and using dense multi- vectors,
    vectors and matrices in parallel.
    
    The Epetra.MultiVector class enables the construction and use of real-
    valued, double- precision dense vectors, multi-vectors, and matrices in a
    distributed memory environment. The dimensions and distribution of the dense
    multi-vectors is determined in part by a Epetra.Comm object, a Epetra.Map
    (or Epetra.LocalMap or Epetra.BlockMap) and the number of vectors passed to
    the constructors described below.
    
    There are several concepts that important for understanding the
    Epetra.MultiVector class:
    
    Multi-vectors, Vectors and Matrices. Vector - A list of real-valued,
    double-precision numbers. Also a multi-vector with one vector.
    
    Multi-Vector - A collection of one or more vectors, all having the same
    length and distribution.
    
    (Dense) Matrix - A special form of multi-vector such that stride in memory
    between any two consecutive vectors in the multi-vector is the same for all
    vectors. This is identical to a two-dimensional array in Fortran and plays
    an important part in high performance computations.
    
    Distributed Global vs. Replicated Local. Distributed Global Multi- vectors -
    In most instances, a multi-vector will be partitioned across multiple memory
    images associated with multiple processors. In this case, there is a unique
    copy of each element and elements are spread across all processors specified
    by the Epetra.Comm communicator.
    
    Replicated Local Multi-vectors - Some algorithms use multi-vectors that are
    too small to be distributed across all processors, the Hessenberg matrix in
    a GMRES computation. In other cases, such as with block iterative methods,
    block dot product functions produce small dense matrices that are required
    by all processors. Replicated local multi-vectors handle these types of
    situation.
    
    Multi-vector Functions vs. Dense Matrix Functions. Multi-vector functions -
    These functions operate simultaneously but independently on each vector in
    the multi-vector and produce individual results for each vector.
    
    Dense matrix functions - These functions operate on the multi-vector as a
    matrix, providing access to selected dense BLAS and LAPACK operations.

    In the python implementation, the MultiVector stores an underlying numpy
    array, with which it shares the data buffer.  This underlying numpy array
    has at least two dimensions, and the first dimension corresponds to the
    number of vectors.  Also, almost all numpy array methods and operators are
    supported.
    """
    def __init__(self, *args):
        """
        __init__(self, BlockMap map, int numVectors,
             bool zeroOut=True) -> MultiVector
        __init__(self, MultiVector source) -> MultiVector
        __init__(self, BlockMap map, PyObject array) -> MultiVector
        __init__(self, DataAccess CV, MultiVector source) -> MultiVector
        __init__(self, DataAccess CV, MultiVector source,
             PyObject range) -> MultiVector
        __init__(self, PyObject array) -> MultiVector

        Arguments:
            map         - BlockMap describing domain decomposition
            numVectors  - Number of vectors
            zeroOut     - Flag controlling whether to initialize MultiVector to
                          zero
            source      - Source MultiVector for copy constructors
            array       - Python sequence that can be converted to a numpy array
                          of doubles for initialization
            CV          - Epetra.Copy or Epetra.View
            range       - Python sequence specifying range of vector indexes
        """
        NumPyMultiVector.__init__(self, *args)
        self.__initArray__()
    def __initArray__(self):
        """
        __initArray__(self)

        Initialize the underlying numpy array.
        """
        UserArray.__init__(self, self.ExtractView(), dtype="d", copy=False)
    def __str__(self):
        """
        __str__(self) -> string

        Return the numpy-style string representation of the MultiVector.
        """
        return str(self.array)
    def __lt__(self,other):
        """
        __lt__(self, other) -> bool

        Less-than operator (<).
        """
        return numpy.less(self.array,other)
    def __le__(self,other):
        """
        __le__(self, other) -> bool

        Less-than-or-equal operator (<=).
        """
        return numpy.less_equal(self.array,other)
    def __eq__(self,other):
        """
        __eq__(self, other) -> bool

        Equal operator (==).
        """
        return numpy.equal(self.array,other)
    def __ne__(self,other):
        """
        __ne__(self, other) -> bool

        Not-equal operator (!=).
        """
        return numpy.not_equal(self.array,other)
    def __gt__(self,other):
        """
        __gt__(self, other) -> bool

        Greater-than operator (>).
        """
        return numpy.greater(self.array,other)
    def __ge__(self,other):
        """
        __ge__(self, other) -> bool

        Greater-than or equal operator (>=).
        """
        return numpy.greater_equal(self.array,other)
    def __getattr__(self, key):
        # This should get called when the MultiVector is accessed after not
        # properly being initialized
        if not "array" in self.__dict__:
            self.__initArray__()
        try:
            return self.array.__getattribute__(key)
        except AttributeError:
            return MultiVector.__getattribute__(self, key)
    def __setattr__(self, key, value):
        "Handle 'this' properly and protect the 'array' and 'shape' attributes"
        if key == "this":
            NumPyMultiVector.__setattr__(self, key, value)
        else:
            if key == "array":
                if key in self.__dict__:
                    raise AttributeError, \
                          "Cannot change Epetra.MultiVector array attribute"
            elif key == "shape":
                value = tuple(value)
                if len(value) < 2:
                    raise ValueError, "Epetra.MultiVector shape is " + \
                          str(value) + " but must have minimum of 2 elements"
            UserArray.__setattr__(self, key, value)
_Epetra.NumPyMultiVector_swigregister(MultiVector)
%}

////////////////////////////////
// Epetra_NumPyVector support //
////////////////////////////////
%rename(NumPyVector) Epetra_NumPyVector;
%epetra_numpy_ctor_exception(Epetra_NumPyVector)
%include "Epetra_NumPyVector.h"
%pythoncode
%{
class Vector(UserArray,NumPyVector):
    """
    Epetra.Vector: A class for constructing and using dense vectors on a
    parallel computer.
    
    The Epetra.Vector class enables the construction and use of real- valued,
    double- precision dense vectors in a distributed memory environment. The
    distribution of the dense vector is determined in part by a Epetra.Comm
    object and a Epetra.Map (or Epetra.LocalMap or Epetra.BlockMap).
    
    This class is derived from the Epetra.MultiVector class. As such, it has
    full access to all of the functionality provided in the Epetra.MultiVector
    class.
    
    Distributed Global vs. Replicated Local Distributed Global Vectors - In most
    instances, a multi-vector will be partitioned across multiple memory images
    associated with multiple processors. In this case, there is a unique copy of
    each element and elements are spread across all processors specified by the
    Epetra.Comm communicator.
    
    Replicated Local Vectors - Some algorithms use vectors that are too small to
    be distributed across all processors. Replicated local vectors handle these
    types of situation.

    In the python implementation, the Vector stores an underlying numpy array,
    with which it shares the data buffer.  Also, almost all numpy array methods
    and operators are supported.
    """
    def __init__(self, *args):
        """
        __init__(self, BlockMap map, bool zeroOut=True) -> Vector
        __init__(self, Vector source) -> Vector
        __init__(self, BlockMap map, PyObject array) -> Vector
        __init__(self, DataAccess CV, Vector source) -> Vector
        __init__(self, DataAccess CV, MultiVector source, PyObject index) -> Vector
        __init__(self, PyObject array) -> Vector

        Arguments:
            map         - BlockMap describing domain decomposition
            zeroOut     - Flag controlling whether to initialize MultiVector to
                          zero
            source      - Source Vector or MultiVector for copy constructors
            array       - Python sequence that can be converted to a numpy array
                          of doubles for initialization
            CV          - Epetra.Copy or Epetra.View
            index       - MultiVector vector index for copy constructor
        """
        NumPyVector.__init__(self, *args)
        self.__initArray__()
    def __initArray__(self):
        """
        __initArray__(self)

        Initialize the underlying numpy array.
        """
        UserArray.__init__(self, self.ExtractView(), dtype="d", copy=False)
    def __str__(self):
        """
        __str__(self) -> string

        Return the numpy-style string representation of the MultiVector.
        """
        return str(self.array)
    def __lt__(self,other):
        """
        __lt__(self, other) -> bool

        Less-than operator (<).
        """
        return numpy.less(self.array,other)
    def __le__(self,other):
        """
        __le__(self, other) -> bool

        Less-than-or-equal operator (<=).
        """
        return numpy.less_equal(self.array,other)
    def __eq__(self,other):
        """
        __eq__(self, other) -> bool

        Equal operator (==).
        """
        return numpy.equal(self.array,other)
    def __ne__(self,other):
        """
        __ne__(self, other) -> bool

        Not-equal operator (!=).
        """
        return numpy.not_equal(self.array,other)
    def __gt__(self,other):
        """
        __gt__(self, other) -> bool

        Greater-than operator (>).
        """
        return numpy.greater(self.array,other)
    def __ge__(self,other):
        """
        __ge__(self, other) -> bool

        Greater-than or equal operator (>=).
        """
        return numpy.greater_equal(self.array,other)
    def __getattr__(self, key):
        # This should get called when the Vector is accessed after not properly
        # being initialized
        if not "array" in self.__dict__:
            self.__initArray__()
        try:
            return self.array.__getattribute__(key)
        except AttributeError:
            return Vector.__getattribute__(self, key)
    def __setattr__(self, key, value):
        "Handle 'this' properly and protect the 'array' attribute"
        if key == "this":
            NumPyVector.__setattr__(self, key, value)
        else:
            if key == "array":
                if key in self.__dict__:
                    raise AttributeError, \
                          "Cannot change Epetra.Vector array attribute"
            UserArray.__setattr__(self, key, value)
_Epetra.NumPyVector_swigregister(Vector)
%}

////////////////////////////////
// Epetra_NumPyFEVector support //
////////////////////////////////
%rename(NumPyFEVector) Epetra_NumPyFEVector;
%epetra_numpy_ctor_exception(Epetra_NumPyFEVector)
%include "Epetra_NumPyFEVector.h"
%pythoncode
%{
class FEVector(UserArray,NumPyFEVector):
    """
    Epetra Finite-Element Vector. This class inherits Epetra.MultiVector and
    thus provides all Epetra.MultiVector functionality, with one restriction:
    currently an Epetra.FEVector only has 1 internal vector.
    
    The added functionality provided by Epetra.FEVector is the ability to
    perform finite-element style vector assembly. It accepts sub-vector
    contributions, such as those that would come from element-load vectors,
    etc., and these sub-vectors need not be wholly locally owned.  In other
    words, the user can assemble overlapping data (e.g., corresponding to shared
    finite-element nodes). When the user is finished assembling their vector
    data, they then call the method Epetra.FEVector::GlobalAssemble() which
    gathers the overlapping data (all non-local data that was input on each
    processor) into the data- distribution specified by the map that the
    Epetra.FEVector is constructed with.
    
    Note: At the current time (Sept 6, 2002) the methods in this implementation
    assume that there is only 1 point associated with each map element. This
    limitation will be removed in the near future.

    In the python implementation, the FEVector stores an underlying numpy array,
    with which it shares the data buffer.  Also, almost all numpy array methods
    and operators are supported.
    """
    def __init__(self, *args):
        """
        __init__(self, BlockMap map, bool zeroOut=True) -> FEVector
        __init__(self, FEVector source) -> FEVector
        __init__(self, BlockMap map, PyObject array) -> FEVector
        __init__(self, DataAccess CV, Vector source) -> FEVector
        __init__(self, DataAccess CV, MultiVector source, PyObject index) -> FEVector
        __init__(self, PyObject array) -> FEVector

        Arguments:
            map         - BlockMap describing domain decomposition
            zeroOut     - Flag controlling whether to initialize MultiVector to
                          zero
            source      - Source Vector or MultiVector for copy constructors
            array       - Python sequence that can be converted to a numpy array
                          of doubles for initialization
            CV          - Epetra.Copy or Epetra.View
            index       - MultiVector vector index for copy constructor
        """
        NumPyFEVector.__init__(self, *args)
        self.__initArray__()
    def __initArray__(self):
        """
        __initArray__(self)

        Initialize the underlying numpy array.
        """
        UserArray.__init__(self, self.ExtractView(), dtype="d", copy=False)
    def __str__(self):
        """
        __str__(self) -> string

        Return the numpy-style string representation of the MultiVector.
        """
        return str(self.array)
    def __lt__(self,other):
        """
        __lt__(self, other) -> bool

        Less-than operator (<).
        """
        return numpy.less(self.array,other)
    def __le__(self,other):
        """
        __le__(self, other) -> bool

        Less-than-or-equal operator (<=).
        """
        return numpy.less_equal(self.array,other)
    def __eq__(self,other):
        """
        __eq__(self, other) -> bool

        Equal operator (==).
        """
        return numpy.equal(self.array,other)
    def __ne__(self,other):
        """
        __ne__(self, other) -> bool

        Not-equal operator (!=).
        """
        return numpy.not_equal(self.array,other)
    def __gt__(self,other):
        """
        __gt__(self, other) -> bool

        Greater-than operator (>).
        """
        return numpy.greater(self.array,other)
    def __ge__(self,other):
        """
        __ge__(self, other) -> bool

        Greater-than or equal operator (>=).
        """
        return numpy.greater_equal(self.array,other)
    def __getattr__(self, key):
        # This should get called when the FEVector is accessed after not properly
        # being initialized
        if not "array" in self.__dict__:
            self.__initArray__()
        try:
            return self.array.__getattribute__(key)
        except AttributeError:
            return FEVector.__getattribute__(self, key)
    def __setattr__(self, key, value):
        "Handle 'this' properly and protect the 'array' attribute"
        if key == "this":
            NumPyFEVector.__setattr__(self, key, value)
        else:
            if key == "array":
                if key in self.__dict__:
                    raise AttributeError, "Cannot change Epetra.FEVector array attribute"
            UserArray.__setattr__(self, key, value)
_Epetra.NumPyFEVector_swigregister(FEVector)
%}
