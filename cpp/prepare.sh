#!/bin/bash

#CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}"  )" >/dev/null && pwd  )"
CUR_DIR="$( pwd )"
LOGICAL_CPU=$(cat /proc/cpuinfo |grep "processor"|wc -l)
PARALLEL=$LOGICAL_CPU

if [ ! -d $CUR_DIR/3rd_party/cmake-3.12.2 ]; then
  cd $CUR_DIR/3rd_party/
  tar -xf cmake-3.12.2.tar.gz
fi

if [ ! -f $CUR_DIR/lib/bin/cmake ]; then
  cd $CUR_DIR/3rd_party/cmake-3.12.2/
  ./bootstrap --prefix=$CUR_DIR/lib --parallel=$PARALLEL && make -j$PARALLEL && make install
fi
export PATH=$CUR_DIR/lib/bin:$PATH

cd $CUR_DIR/3rd_party
LLVM_VERSION=6.0.1
unzip_llvm_tar(){
  if [ ! -d $1-$LLVM_VERSION.src ]; then
    tar -xf $1-$LLVM_VERSION.src.tar.xz
  fi
}
unzip_llvm_tar llvm
unzip_llvm_tar cfe
unzip_llvm_tar clang-tools-extra
unzip_llvm_tar compiler-rt
unzip_llvm_tar libcxx
unzip_llvm_tar libcxxabi
unzip_llvm_tar libunwind
unzip_llvm_tar lld

if [ ! -d $CUR_DIR/lib/include/llvm ]; then
  ln -s $CUR_DIR/3rd_party/cfe-$LLVM_VERSION.src $CUR_DIR/3rd_party/llvm-$LLVM_VERSION.src/tools/clang
  ln -s $CUR_DIR/3rd_party/lld-$LLVM_VERSION.src $CUR_DIR/3rd_party/llvm-$LLVM_VERSION.src/tools/lld
  ln -s $CUR_DIR/3rd_party/clang-tools-extra-$LLVM_VERSION.src $CUR_DIR/3rd_party/llvm-$LLVM_VERSION.src/tools/clang/tools/extra
  ln -s $CUR_DIR/3rd_party/compiler-rt-$LLVM_VERSION.src $CUR_DIR/3rd_party/llvm-$LLVM_VERSION.src/projects/compiler-rt
  ln -s $CUR_DIR/3rd_party/libcxx-$LLVM_VERSION.src $CUR_DIR/3rd_party/llvm-$LLVM_VERSION.src/projects/libcxx
  ln -s $CUR_DIR/3rd_party/libcxxabi-$LLVM_VERSION.src $CUR_DIR/3rd_party/llvm-$LLVM_VERSION.src/projects/libcxxabi

  cd $CUR_DIR/3rd_party
  #rm -rf llvm-build
  mkdir llvm-build
  cd llvm-build
  cmake -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_EH=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CUR_DIR/lib/ ../llvm-$LLVM_VERSION.src
  make CC=clang -j$PARALLEL && make install
fi

export CXX=$CUR_DIR/lib/bin/clang++
export CC=$CUR_DIR/lib/bin/clang

cd $CUR_DIR/3rd_party
if [ ! -d "boost_1_67_0"  ]; then
  tar -zxvf boost_1_67_0.tar.gz
fi
if [ ! -d $CUR_DIR/lib/include/boost ]; then
  cd boost_1_67_0
  ./bootstrap.sh --prefix=$CUR_DIR/lib/
  ./b2 --toolset=clang
  ./b2 install
fi

build_with_cmake(){
  cd $CUR_DIR/3rd_party/$1
  if [ -d "build" ]; then
    rm -rf build
  fi
  mkdir build
  cd build
  cmake -DCMAKE_MODULE_PATH=$CUR_DIR/lib/lib/cmake -DCMAKE_LIBRARY_PATH=$CUR_DIR/lib/lib -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CUR_DIR/lib/ ../
  make -j$PARALLEL && make install && make clean
  cd ../ && rm -rf build
}

build_with_configure(){
  cd $CUR_DIR/3rd_party/$1
  ./configure --prefix=$CUR_DIR/lib/
  make -j$PARALLEL && make install && make clean
}

build_with_make(){
  cd $CUR_DIR/3rd_party/$1
  make -j$PARALLEL && make install PREFIX=$CUR_DIR/lib/
}

if [ ! -d $CUR_DIR/lib/include/glog/ ]; then
  build_with_cmake glog
fi

if [ ! -d $CUR_DIR/lib/include/gtest/ ]; then
  build_with_cmake googletest
fi

if [ ! -d $CUR_DIR/lib/include/gflags ]; then
  build_with_cmake gflags
fi

if [ ! -d $CUR_DIR/lib/include/velocypack ]; then
  build_with_cmake velocypack
fi

if [ ! -f $CUR_DIR/lib/include/http_parser.h ]; then
  build_with_make http-parser
fi

if [ ! -d $CUR_DIR/lib/include/fuerte ]; then
  build_with_cmake fuerte
fi

