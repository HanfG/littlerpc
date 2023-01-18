
add_executable(local_rpc "./test.pb-c.c" "./local_rpc.cpp")

target_link_libraries(local_rpc protobuf-c LittleRPC)