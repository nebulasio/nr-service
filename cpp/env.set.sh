
CUR_DIR="$( pwd )"
export AR=$CUR_DIR/lib/bin/llvm-ar
export CC=$CUR_DIR/lib/bin/clang
export CXX=$CUR_DIR/lib/bin/clang++
export PATH=$CUR_DIR/lib/bin:$PATH
export LD_LIBRARY_PATH=$CUR_DIR/lib/lib:$LD_LIBRARY_PATH

# arangodb url, user, passwd, db
export DB_URL=
export DB_USER_NAME=
export DB_PASSWORD=
export NEBULAS_DB=

