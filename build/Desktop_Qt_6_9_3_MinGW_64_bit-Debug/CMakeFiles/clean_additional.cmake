# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\yyy_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\yyy_autogen.dir\\ParseCache.txt"
  "yyy_autogen"
  )
endif()
