#!/bin/bash
rm -rf CMakeCache.txt CMakeFiles
cmake \
-D CMAKE_INSTALL_PREFIX="/home/rppawlo/install/gnu4.8.1/pike" \
-D Trilinos_EXTRA_REPOSITORIES="pike" \
-D Trilinos_ENABLE_ALL_PACKAGES:BOOL=OFF \
-D Trilinos_ENABLE_ALL_OPTIONAL_PACKAGES:BOOL=ON \
-D Trilinos_ENABLE_TESTS:BOOL=OFF \
-D Trilinos_ENABLE_PikeBlackBox:BOOL=ON \
-D Trilinos_ENABLE_PikeImplicit:BOOL=OFF \
-D Teuchos_ENABLE_LONG_LONG_INT:BOOL=ON \
-D Trilinos_ENABLE_DEBUG=OFF \
-D Pike_ENABLE_DEBUG:BOOL=ON \
-D Pike_ENABLE_TESTS:BOOL=ON \
-D Trilinos_ENABLE_TEUCHOS_TIME_MONITOR:BOOL=OFF \
-D Pike_ENABLE_TEUCHOS_TIME_MONITOR:BOOL=ON \
-D TPL_BLAS_LIBRARIES:PATH="/home/rppawlo/install/gnu4.8.1/blas/libblas.a" \
-D TPL_LAPACK_LIBRARIES:PATH="/home/rppawlo/install/gnu4.8.1/lapack/liblapack.a" \
-D TPL_ENABLE_MPI:BOOL=ON \
-D MPI_BASE_DIR:PATH="/home/rppawlo/install/gnu4.8.1/openmpi" \
      -D TPL_ENABLE_Boost:BOOL=ON \
      -D Boost_INCLUDE_DIRS:FILEPATH="/home/rppawlo/install/gnu4.8.1/boost/include" \
      -D TPL_ENABLE_BoostLib:BOOL=ON \
      -D BoostLib_INCLUDE_DIRS:FILEPATH="/home/rppawlo/install/gnu4.8.1/boost/include" \
      -D BoostLib_LIBRARY_DIRS:FILEPATH="/home/rppawlo/install/gnu4.8.1/boost/lib/" \
-D MPI_EXEC_MAX_NUMPROCS:STRING=6 \
-D CMAKE_CXX_COMPILER:FILEPATH="/home/rppawlo/install/gnu4.8.1/openmpi/bin/mpiCC" \
-D CMAKE_C_COMPILER:FILEPATH="/home/rppawlo/install/gnu4.8.1/openmpi/bin/mpicc" \
-D CMAKE_Fortran_COMPILER:FILEPATH="/home/rppawlo/install/gnu4.8.1/openmpi/bin/mpif77" \
-D Trilinos_ENABLE_Fortran:BOOL=ON \
-D CMAKE_CXX_FLAGS:STRING="-g -Wall" \
-D CMAKE_C_FLAGS:STRING="" \
-D CMAKE_VERBOSE_MAKEFILE:BOOL=OFF \
-D Trilinos_VERBOSE_CONFIGURE:BOOL=OFF \
-D CMAKE_SKIP_RULE_DEPENDENCY=ON \
-D CMAKE_BUILD_TYPE:STRING=DEBUG \
-D Trilinos_ENABLE_EXPLICIT_INSTANTIATION:BOOL=OFF \
-D Trilinos_ENABLE_CHECKED_STL:BOOL=OFF \
-D Trilinos_ENABLE_INSTALL_CMAKE_CONFIG_FILES:BOOL=OFF \
../Trilinos 
