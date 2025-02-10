cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(sandbox C)

if(NOT DEFINED Python3_FIND_VIRTUALENV)
  set(Python3_FIND_VIRTUALENV FIRST)
endif()

find_package (Python3 COMPONENTS Interpreter Development)

if (NOT Python3_Development_FOUND)
  message(FATAL_ERROR "Python3 include directory not found!")
endif()


Python3_add_library(_sandbox MODULE "src/sandbox.c" "src/tokenizer.c")

install(TARGETS _sandbox
        EXCLUDE_FROM_ALL
        COMPONENT python_modules
        DESTINATION ${PY_BUILD_CMAKE_IMPORT_NAME})
