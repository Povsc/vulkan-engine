find_program(SLANGC_EXECUTABLE slangc HINTS "$ENV{VULKAN_SDK}/bin" REQUIRED)

# Compiles each .slang file to <name>.spv in OUTPUT_DIR. Without -entry, slangc emits every
# entry point marked [shader("...")], so one file can hold vertMain/fragMain/compMain.
function(add_slang_shaders TARGET)
    cmake_parse_arguments(ARG "" "OUTPUT_DIR" "SOURCES" ${ARGN})

    set(outputs)
    foreach(src IN LISTS ARG_SOURCES)
        get_filename_component(name "${src}" NAME_WE)
        set(spv "${ARG_OUTPUT_DIR}/${name}.spv")
        add_custom_command(
            OUTPUT "${spv}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${ARG_OUTPUT_DIR}"
            COMMAND "${SLANGC_EXECUTABLE}" "${src}"
                    -target spirv -profile spirv_1_4 -emit-spirv-directly
                    -fvk-use-entrypoint-name -g
                    -o "${spv}"
            DEPENDS "${src}"
            COMMENT "slangc ${name}.slang"
            VERBATIM
        )
        list(APPEND outputs "${spv}")
    endforeach()

    add_custom_target(${TARGET}_shaders DEPENDS ${outputs})
    add_dependencies(${TARGET} ${TARGET}_shaders)
endfunction()