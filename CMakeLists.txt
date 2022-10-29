CMAKE_MINIMUM_REQUIRED(VERSION 3.24.2)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake")

project("basic-vscode-stm32-cmake" C ASM)

set(CMAKE_C_STANDARD "11")

set(linker_script "${CMAKE_CURRENT_LIST_DIR}/STM32F401CCUX_FLASH.ld")
set(device_name "STM32F401xC")

set(c_compile_options
	# CPU specific
	"-mcpu=cortex-m4"
	"-mfloat-abi=soft"
	"-mthumb"
	# Non-CPU specific
	"--specs=nano.specs"
	"-ffunction-sections"
	"-fdata-sections"
	"-fstack-usage"
	# Other options
	"-g3"
	"-c"
	"-Os"
	"-Wall"
)
	
set(compile_definitions "${device_name}")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	list(APPEND compile_definitions "DEBUG" "_DEBUG")
else()
	list(APPEND compile_definitions "NDEBUG")
endif()

set(link_flags
	"-mcpu=cortex-m4 \
	-mfloat-abi=soft \
	-T\"${CMAKE_CURRENT_LIST_DIR}/STM32F401CCUX_FLASH.ld\" \
	-Wl,-V \
	-Wl,-Map=\"${PROJECT_NAME}.map\" \
	-Wl,--gc-sections \
	-static \
	--specs=nano.specs \
	-Wl,--start-group -lc -lm -Wl,--end-group"
)
add_compile_options("${c_compile_options}")
add_compile_definitions("${compile_definitions}")

include_directories("CMSIS/Inc/")
include_directories("Inc/")
include_directories("Startup/")

file (GLOB_RECURSE startup "Startup/*.[s]")
file(GLOB_RECURSE sources "Src/*.[c]")
file(GLOB_RECURSE cmsis "CMSIS/Src/*.[c]")

add_executable("${PROJECT_NAME}" ${cmsis} ${startup} ${sources})
set_target_properties("${PROJECT_NAME}" PROPERTIES LINK_FLAGS "${link_flags}")

# The OBJCOPY utility will separate the EEPROM section from the main program
add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
	COMMAND "${CMAKE_OBJCOPY}" "$<TARGET_FILE:${PROJECT_NAME}>" -O ihex "program.hex"
	COMMAND "${CMAKE_OBJCOPY}" "$<TARGET_FILE:${PROJECT_NAME}>" "program.elf"
	BYPRODUCTS "program.hex" "program.elf"
)