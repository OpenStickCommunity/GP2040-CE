function (patch_board_config)
	find_package(Python3 REQUIRED COMPONENTS Interpreter)

	set(VENV ${CMAKE_CURRENT_BINARY_DIR}/venv)
	if(CMAKE_HOST_WIN32)
		set(VENV_BIN_DIR ${VENV}/Scripts)
	else()
		set(VENV_BIN_DIR ${VENV}/bin)
	endif()

	set(PROTO_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)

	add_custom_target(
		PatchBoardConfig ALL
		DEPENDS ${PROJECT_NAME}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		# something in the protobuf compiler doesn't like the multiple paths,
		# so we'll copy all the .proto files into one location
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROTO_OUTPUT_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/nanopb/generator/proto/nanopb.proto ${PROTO_OUTPUT_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/proto/enums.proto ${PROTO_OUTPUT_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/proto/config.proto ${PROTO_OUTPUT_DIR}
		COMMAND ${VENV_BIN_DIR}/concatenate
			-P ${PROTO_OUTPUT_DIR}
			./build/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.bin
			--json-board-config-filename ./configs/${GP2040_BOARDCONFIG}/board-config.json
			--backup
			--new-filename ./build/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.uf2
		COMMAND ${VENV_BIN_DIR}/summarize-gp2040ce
			--filename ./build/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.uf2
		COMMENT "Patching binary with board config"
	)
endfunction()
