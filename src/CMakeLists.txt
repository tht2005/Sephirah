file(GLOB LIB_SRCS
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER LIB_SRCS EXCLUDE REGEX "main\\.cpp$")

add_library(sephirah_lib STATIC ${LIB_SRCS})

add_executable(sephirah main.cpp)
target_link_libraries(sephirah PRIVATE sephirah_lib)
