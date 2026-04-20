# HelloCustomWidgets: uses two-level APP_SUB (category/widget_name)
# Usage: cmake -DAPP=HelloCustomWidgets -DAPP_SUB=input/auto_suggest_box ..

# Common source files (uicode_disp0.c multi-display entry)
file(GLOB HELLO_CUSTOM_SRC *.c)

# Sub-app source files
if(NOT APP_SUB STREQUAL "")
    file(GLOB APP_SUB_SRC ${APP_SUB}/*.c)
    file(GLOB_RECURSE APP_SUB_RESOURCE_SRC ${APP_SUB}/resource/*.c)
    list(APPEND HELLO_CUSTOM_SRC ${APP_SUB_SRC} ${APP_SUB_RESOURCE_SRC})
endif()

add_library(example_lib ${HELLO_CUSTOM_SRC})
target_include_directories(example_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
