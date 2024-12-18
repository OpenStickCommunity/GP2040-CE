function (compile_proto)
	find_package(Python3 REQUIRED COMPONENTS Interpreter)

	set(VENV ${CMAKE_CURRENT_BINARY_DIR}/venv)
	set(VENV_FILE ${VENV}/environment.txt)
	if(CMAKE_HOST_WIN32)
		set(VENV_BIN_DIR ${VENV}/Scripts)
	else()
		set(VENV_BIN_DIR ${VENV}/bin)
	endif()

	add_custom_command(
		DEPENDS ${CMAKE_SOURCE_DIR}/lib/nanopb/extra/requirements.txt
		COMMAND ${Python3_EXECUTABLE} -m venv ${VENV}
		COMMAND ${VENV_BIN_DIR}/pip --disable-pip-version-check install -r ${CMAKE_SOURCE_DIR}/lib/nanopb/extra/requirements.txt
		COMMAND ${VENV_BIN_DIR}/pip --disable-pip-version-check install gp2040ce-binary-tools
		COMMAND ${VENV_BIN_DIR}/pip freeze > ${VENV_FILE}
		OUTPUT ${VENV_FILE}
		COMMENT "Setting up Python Virtual Environment"
	)

	set(NANOPB_GENERATOR ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/nanopb_generator.py)
	set(PYTHON_GENERATOR ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/protoc)
	set(PROTO_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
	set(PROTO_OUTPUT_DIR ${PROTO_OUTPUT_DIR} PARENT_SCOPE)

	add_custom_command(
		DEPENDS ${VENV_FILE} ${NANOPB_GENERATOR} ${CMAKE_SOURCE_DIR}/proto/enums.proto ${CMAKE_SOURCE_DIR}/proto/config.proto ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto/nanopb.proto
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROTO_OUTPUT_DIR}
		COMMAND ${VENV_BIN_DIR}/python ${NANOPB_GENERATOR}
			-q
			-D ${PROTO_OUTPUT_DIR}
			-I ${CMAKE_SOURCE_DIR}/proto
			-I ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto
			${CMAKE_SOURCE_DIR}/proto/enums.proto
		COMMAND ${VENV_BIN_DIR}/python ${NANOPB_GENERATOR}
			-q
			-D ${PROTO_OUTPUT_DIR}
			-I ${CMAKE_SOURCE_DIR}/proto
			-I ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto
			${CMAKE_SOURCE_DIR}/proto/config.proto
		OUTPUT ${PROTO_OUTPUT_DIR}/config.pb.c ${PROTO_OUTPUT_DIR}/config.pb.h ${PROTO_OUTPUT_DIR}/enums.pb.c ${PROTO_OUTPUT_DIR}/enums.pb.h
		COMMENT "Compiling enums.proto and config.proto"
	)

	add_custom_command(
		DEPENDS ${VENV_FILE} ${PYTHON_GENERATOR} ${CMAKE_SOURCE_DIR}/proto/enums.proto ${CMAKE_SOURCE_DIR}/proto/config.proto ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto/nanopb.proto
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROTO_OUTPUT_DIR}
		COMMAND ${VENV_BIN_DIR}/python ${PYTHON_GENERATOR}
			--python_out ${PROTO_OUTPUT_DIR}
			-I ${CMAKE_SOURCE_DIR}/proto
			-I ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto
			${CMAKE_SOURCE_DIR}/proto/enums.proto
		COMMAND ${VENV_BIN_DIR}/python ${PYTHON_GENERATOR}
			--python_out ${PROTO_OUTPUT_DIR}
			-I ${CMAKE_SOURCE_DIR}/proto
			-I ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto
			${CMAKE_SOURCE_DIR}/proto/config.proto
		COMMAND ${VENV_BIN_DIR}/python ${PYTHON_GENERATOR}
			--python_out ${PROTO_OUTPUT_DIR}
			-I ${CMAKE_SOURCE_DIR}/proto
			-I ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto
			${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto/nanopb.proto
		OUTPUT ${PROTO_OUTPUT_DIR}/config_pb2.py ${PROTO_OUTPUT_DIR}/enums_pb2.py ${PROTO_OUTPUT_DIR}/nanopb_pb2.py
		COMMENT "Compiling config_pb2.py, enums_pb2.py, and nanopb_pb2.py Python modules"
	)
endfunction()
