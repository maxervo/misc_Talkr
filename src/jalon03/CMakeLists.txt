cmake_minimum_required(VERSION 2.8.3)

project(RE216_JALON03)

#add here source files you need to compile and link to the client app
SET(SRC_CLIENT
	common.c
	client.c
	)


add_executable(RE216_JALON03_CLIENT ${SRC_CLIENT})


#add here source files you need to compile and link to the server app
SET(SRC_SERVER
	common.c
        server.c)


add_executable(RE216_JALON03_SERVER  ${SRC_SERVER})


