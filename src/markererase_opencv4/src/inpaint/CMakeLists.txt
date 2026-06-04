
aux_source_directory(. INPAINT_SRCS)

MESSAGE(STATUS "building inpaint")

add_library (inpaint STATIC ${INPAINT_SRCS})

target_link_libraries(inpaint mrcimg)

set_target_properties(inpaint PROPERTIES OUTPUT_NAME "inpaint")
set_target_properties(inpaint PROPERTIES CLEAN_DIRECT_OUTPUT 1)