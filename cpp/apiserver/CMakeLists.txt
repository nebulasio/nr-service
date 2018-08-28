pybind11_add_module(nebserver apiserver.cpp)

target_link_libraries(nebserver ${common_libs})
