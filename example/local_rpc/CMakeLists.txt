add_definitions(-DLITTLE_RPC_USER_CONF)

include_directories("./")

add_executable(local_rpc "./test.pb-c.c" "role1.c" "role2.c" "./local_rpc.c")

target_link_libraries(local_rpc protobuf-c LittleRPC)