function (patch_board_config)
	find_package(Python3 REQUIRED COMPONENTS Interpreter)

	set(VENV ${CMAKE_CURRENT_BINARY_DIR}/venv)
	if(CMAKE_HOST_WIN32)
		set(VENV_BIN_DIR ${VENV}/Scripts)
	else()
		set(VENV_BIN_DIR ${VENV}/bin)
	endif()

	add_custom_target(
		PatchBoardConfig ALL
		DEPENDS ${PROJECT_NAME}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMAND ${VENV_BIN_DIR}/concatenate
			-P ./proto
			-P ./lib/nanopb/generator/proto
			./build/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.bin
			--json-board-config-filename ./configs/${GP2040_BOARDCONFIG}/board-config.json
			--new-filename ./build/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.uf2
		COMMAND ${VENV_BIN_DIR}/concatenate
			-P ./proto
			-P ./lib/nanopb/generator/proto
			./build/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.bin
			--json-board-config-filename ./configs/${GP2040_BOARDCONFIG}/board-config.json
			--new-filename ./build/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.bin
		COMMENT "Patching binary with board config"
	)
endfunction()
