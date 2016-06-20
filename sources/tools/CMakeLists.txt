PROJECT(TOOLS)

SET(TOOLS
    dbitq_save
    dbitq_loads
    itqlsh_test
    create_benchmark
    create_benchmark_filedb
)

FOREACH(TOOL ${TOOLS})
    ADD_EXECUTABLE(${TOOL} ${TOOL}.cpp)
ENDFOREACH(TOOL)

SET(EXECUTABLE_OUTPUT_PATH ${LSHBOX_BINARY_DIR}/bin/${SAVE_CLASS})