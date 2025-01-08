set(SRC_LIST main.cc)
add_executable(main ${SRC_LIST})
add_executable(client client.cc)
target_link_libraries(main net service)

