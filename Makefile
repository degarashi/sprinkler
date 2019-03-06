LIB_NAME			:= sprinkler
MAKE_GDBINIT		:= YES

ADDITIONAL_CMAKE_OPTION		:= \
	-DCMAKE_PREFIX_PATH=/opt/Qt/5.12.0/gcc_64

include lubee/common.make
