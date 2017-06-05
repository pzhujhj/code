cmake_minimum_required(VERSION 2.6)

PROJECT(myubus C)
ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -Wmissing-declarations)

FILE(GLOB SOURCES *.c)

ADD_EXECUTABLE(myubus ${SOURCES})
#TARGET_LINK_LIBRARIES(myubus ubox ubus blobmsg_json)
TARGET_LINK_LIBRARIES(myubus m)

INSTALL(TARGETS myubus
        RUNTIME DESTINATION sbin
)
