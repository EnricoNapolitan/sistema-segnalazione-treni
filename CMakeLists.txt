cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
set (CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set (CMAKE_CXX_FLAGS "-O2")
set (CMAKE_CXX_FLAGS_DEBUG "-O2")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")

project (Rail)

include_directories(src)
add_executable (Rail
  src/main.cpp
  src/Treno/Treno.cpp
  src/Stazione/Stazione.cpp
  src/Stazione/StazioneLocale/StazioneLocale.cpp
  src/Stazione/StazionePrincipale/StazionePrincipale.cpp
  src/TorreDiControllo/TorreDiControllo.cpp
  src/Timetable/Timetable.cpp
 )
configure_file(src/data/line_description.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(src/data/timetables.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
target_link_libraries(Rail
  -pthread
)
