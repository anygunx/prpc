
ADD_DEFINITIONS(-D__STDC_LIMIT_MACROS)

FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)
BISON_TARGET(PRPC_PARSER prpc.y ${CMAKE_CURRENT_BINARY_DIR}/prpc.tab.cc)
IF(MSVC)
	FLEX_TARGET(PRPC_SCANNER prpc.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc COMPILE_FLAGS "--nounistd" )
ELSE()
	FLEX_TARGET(PRPC_SCANNER prpc.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc)
ENDIF()
ADD_FLEX_BISON_DEPENDENCY(PRPC_SCANNER PRPC_PARSER)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../common)

SET(YACC
	${BISON_PRPC_PARSER_OUTPUTS}
	${FLEX_PRPC_SCANNER_OUTPUTS}
	prpc.y
	prpc.l
)

SET(INC
	node.h
	enumer.h
	schema.h
	visiter.h
    scanner.h
	service.h
)
	
SET(SRC
	scanner.cc
	visiter.cc
	enumer.cc
	service.cc
	schema.cc
	visiter.cc.cc
	visiter.cs.cc
	visiter.go.cc
)

SOURCE_GROUP(Yacc FILES ${YACC})
SOURCE_GROUP(Header FILES ${INC})
SOURCE_GROUP(Source FILES ${SRC})

ADD_EXECUTABLE(prpc ${YACC} ${INC} ${SRC})
TARGET_LINK_LIBRARIES(prpc common)
ADD_DEPENDENCIES(prpc common)
TARGET_LINK_LIBRARIES(prpc ${Boost_LIBRARIES})

INSTALL(TARGETS prpc DESTINATION bin)
