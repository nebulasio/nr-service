pybind11_add_module(nebserver apiserver.cpp pybind11_module.cpp)
target_link_libraries(nebserver PRIVATE ${common_libs})

add_executable(lru_cache cache/lru_cache.cpp)
target_link_libraries(lru_cache ${common_libs} gflags)

