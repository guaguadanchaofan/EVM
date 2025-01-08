add_library(net net.cc)
add_library(service service.cc device.cc)
target_include_directories(net PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(service PUBLIC ${CMAKE_SOURCE_DIR}/include)