cmake_minimum_required(VERSION 3.10)

# Set the project name and specify the language
project(sMPI C ASM)

# Compiler options
set(CMAKE_C_FLAGS "-msmall-data-limit=8 -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -march=rv32imc_zicsr -mabi=ilp32")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")

# Source files
set(SOURCE_FILES
    ../src/start.S #Keep this start file at the beginning.
    ../src/main.c
    ../src/common.c
    ../src/intc.c
    ../src/sMPI.c
)

# Include directories
include_directories(
    ../include
)

# Target name
set(TARGET "sMPI")

# Add the executable
add_executable(${TARGET} ${SOURCE_FILES})

# Linker script
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../ldfile/link_GiGa_mod.ld")

# Set the target properties for the linker and output file names
set_target_properties(${TARGET} PROPERTIES
    LINK_FLAGS "-T ${LINKER_SCRIPT} -Xlinker --gc-sections --specs=nosys.specs -nostartfiles"
    OUTPUT_NAME "${TARGET}"
    SUFFIX ""
    PREFIX ""
)

# # Generate HEX file
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -g -O verilog ${TARGET} ${TARGET}.hex
    COMMENT "Creating ${TARGET}.hex file"
)

# # Generate BIN file
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET} ${TARGET}.bin
    COMMENT "Creating ${TARGET}.bin file"
)

# # Generate MAP file
add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMENT "Creating ${TARGET}.map file"
)