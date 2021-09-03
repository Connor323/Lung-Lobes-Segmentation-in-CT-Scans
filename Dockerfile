# ref https://gist.github.com/pangyuteng/e942648579ebfd3c6e07c50eb81f42ac

FROM ubuntu:18.04
    
ARG MYPATH=/usr/local
ARG MYLIBPATH=/usr/lib

RUN apt-get update && apt-get install -y --no-install-recommends \
        autotools-dev \
        build-essential \
        ca-certificates \
        cmake \
        git \
        wget && \
    rm -rf /var/lib/apt/lists/*

# abuse opt directory.
WORKDIR /opt
# install miniconda.
RUN wget --quiet --no-check-certificate https://repo.continuum.io/miniconda/Miniconda3-4.6.14-Linux-x86_64.sh --no-check-certificate -O ~/miniconda.sh && \
    /bin/bash ~/miniconda.sh -b -p /opt/conda
    
# create and activate python virtual env with desired version
RUN /opt/conda/bin/conda create -n py python=3.7.2
RUN echo "source /opt/conda/bin/activate py" > ~/.bashrc
ENV PATH /opt/conda/envs/py/bin:$PATH
RUN /bin/bash -c "source /opt/conda/bin/activate py"

# install cmake
WORKDIR /opt
RUN wget --quiet --no-check-certificate https://github.com/Kitware/CMake/releases/download/v3.15.0-rc4/cmake-3.15.0-rc4-Linux-x86_64.sh
RUN bash cmake-3.15.0-rc4-Linux-x86_64.sh --skip-license --prefix=$MYPATH

RUN mkdir -p /opt/sources

# build zlib
WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate https://zlib.net/zlib-1.2.11.tar.gz -O zlib.tar.gz && \
    tar xfz zlib.tar.gz
WORKDIR /opt/sources/zlib-1.2.11
RUN ./configure --prefix=$MYPATH && make -j"$(nproc)" && make install -j"$(nproc)"

WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate https://github.com/DCMTK/dcmtk/archive/DCMTK-3.6.4.tar.gz -O dcmtk.tar.gz && \
    tar xfz dcmtk.tar.gz
WORKDIR /opt/sources/dcmtk-DCMTK-3.6.4
RUN mkdir build
WORKDIR /opt/sources/dcmtk-DCMTK-3.6.4/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=$MYPATH -DCMAKE_LIBRARY_PATH=$MYLIBPATH
RUN make -j"$(nproc)" && make install -j"$(nproc)"

# build vtk
#likely not needed# RUN sudo apt-get install mesa-common-dev libgl1-mesa-dev freeglut3-devWORKDIR /opt/sources
WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate http://www.vtk.org/files/release/7.0/VTK-7.0.0.tar.gz -O vtk.tar.gz && \
    tar xfz vtk.tar.gz
WORKDIR /opt/sources/VTK-7.0.0
RUN sed -i 's/\[345\]/[34567]/g' CMake/vtkCompilerExtras.cmake
RUN sed -i 's/\[345\]/[34567]/g' CMake/GenerateExportHeader.cmake
RUN mkdir build
WORKDIR /opt/sources/VTK-7.0.0/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=$MYPATH -DCMAKE_BUILD_TYPE=Release \
    -DVTK_Group_Rendering:BOOL=OFF -DVTK_Group_StandAlone:BOOL=ON -DVTK_RENDERING_BACKEND=None
RUN make -j"$(nproc)" && make install -j"$(nproc)"

# build itk
WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate https://github.com/InsightSoftwareConsortium/ITK/releases/download/v5.0.0/InsightToolkit-5.0.0.tar.gz -O itk.tar.gz && \
    tar xfz itk.tar.gz
WORKDIR /opt/sources/InsightToolkit-5.0.0
RUN mkdir build
WORKDIR /opt/sources/InsightToolkit-5.0.0/build
RUN cmake .. -DCMAKE_INSTALL_PREFIX=$MYPATH -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_DOXYGEN=OFF -DBUILD_EXAMPLES=OFF \
    -DBUILD_SHARED_LIBS=OFF -DITK_DYNAMIC_LOADING=OFF -DBUILD_TESTING=OFF \
    -DCMAKE_BACKWARDS_COMPATIBILITY=3.1 -DITK_USE_KWSTYLE=OFF \
    -DITK_BUILD_ALL_MODULES=ON -DModule_ITKVtkGlue=ON -DITK_USE_REVIEW=ON
RUN make -j"$(nproc)" && make install -j"$(nproc)"

# build boost
WORKDIR /opt/sources
RUN wget --quiet --no-check-certificate http://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz -O boost.tar.gz && \
    tar xfz boost.tar.gz
WORKDIR /opt/sources/boost_1_60_0
ENV CPLUS_INCLUDE_PATH /opt/conda/envs/py/include/python3.7m/
RUN ./bootstrap.sh --prefix=$MYPATH --with-python=python3.7 --with-libraries=python,filesystem,system,test
# PATCH for python 3.7 compliance - see https://github.com/boostorg/python/commit/660487c43fde76f3e64f1cb2e644500da92fe582 for detail
RUN wget https://raw.githubusercontent.com/boostorg/python/660487c43fde76f3e64f1cb2e644500da92fe582/src/converter/builtin_converters.cpp
RUN mv libs/python/src/converter/builtin_converters.cpp libs/python/src/converter/builtin_converters.BAK
RUN mv builtin_converters.cpp libs/python/src/converter/builtin_converters.cpp
RUN ./b2 install

RUN /bin/bash -c "source /opt/conda/bin/activate py && conda install cython numpy -y && pip install scikit-build"
RUN /bin/bash -c "source /opt/conda/bin/activate py && pip install scipy scikit-image SimpleITK pydicom notebook"

RUN apt-get update; apt-get install gdb unzip curl -yq

RUN mkdir -p /opt/code
WORKDIR /opt/code

COPY * /opt/code/




