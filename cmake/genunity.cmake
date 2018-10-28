macro(generate_unity_sources)
	set(_OPTIONS_ARGS SOURCES EXECUTABLE LIBRARY WINDOWED)
	set(_ONE_VALUE_ARGS TARGET UNITY_SRC)
	set(_MULTI_VALUE_ARGS SRCS)

	cmake_parse_arguments(_UNITY "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN} )

	if (NOT _UNITY_TARGET)
		message(FATAL_ERROR "generate_unity_sources requires the TARGET argument")
	endif()
	if (NOT _UNITY_SRCS)
		message(FATAL_ERROR "generate_unity_sources requires the SRCS argument")
	endif()
	if (NOT _UNITY_UNITY_SRC)
		set(_UNITY_UNITY_SRC "${CMAKE_CURRENT_BINARY_DIR}/${_UNITY_TARGET}_unity.cpp")
	endif()

	set(TARGET ${_UNITY_TARGET})
	set(UNITY_SRC ${_UNITY_UNITY_SRC})
	set(SRCS ${_UNITY_SRCS})

	get_property(NOUNITY GLOBAL PROPERTY ${TARGET}_NOUNITY)
	if (NOUNITY)
		if (_UNITY_SOURCES)
			target_sources(${TARGET} PRIVATE ${SRCS})
		elseif (_UNITY_EXECUTABLE)
			if (_UNITY_WINDOWED)
				if (WINDOWS)
					add_executable(${TARGET} WIN32 ${SRCS})
					if (USE_MSVC)
						set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
					endif()
				else()
					add_executable(${TARGET} ${SRCS})
				endif()
			else()
				add_executable(${TARGET} ${SRCS})
				if (WINDOWS)
					if (USE_MSVC)
						set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
					endif()
				endif()
			endif()
		elseif (_UNITY_LIBRARY)
			add_library(${TARGET} ${SRCS})
		endif()
	else()
		set(unity_srcs)
		list(APPEND unity_srcs ${UNITY_SRC})
		add_custom_command(
			OUTPUT ${UNITY_SRC}
			COMMAND ${CMAKE_COMMAND} -D "SRCS=\"${SRCS}\"" -D UNITY_SRC="${UNITY_SRC}" -D DIR="${CMAKE_CURRENT_SOURCE_DIR}" -P "${ROOT_DIR}/cmake/GenerateUnity.cmake"
			DEPENDS ${SRCS}
			COMMENT "Generate unity sources for ${TARGET}"
		)
		engine_mark_as_generated(${UNITY_SRC} ${UNITY_SRC}.in)
		foreach(SRC ${SRCS})
			get_filename_component(extension ${SRC} EXT)
			if (NOT "${extension}" STREQUAL ".cpp")
				list(APPEND unity_srcs ${SRC})
				continue()
			endif()
		endforeach()
		if (_UNITY_SOURCES)
			target_sources(${TARGET} PRIVATE ${unity_srcs})
		elseif (_UNITY_EXECUTABLE)
			if (_UNITY_WINDOWED)
				if (WINDOWS)
					add_executable(${TARGET} WIN32 ${unity_srcs})
					if (USE_MSVC)
						set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
					endif()
				else()
					add_executable(${TARGET} ${unity_srcs})
				endif()
			else()
				add_executable(${TARGET} ${unity_srcs})
				if (WINDOWS)
					if (USE_MSVC)
						set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
					endif()
				endif()
			endif()
		elseif (_UNITY_LIBRARY)
			add_library(${TARGET} ${unity_srcs})
		endif()
	endif()
endmacro()