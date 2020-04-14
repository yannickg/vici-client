include_directories (${siptricks_SOURCE_DIR}) 

file(GLOB Sources *.cpp)

add_executable (b2bua ${Sources}) 

target_link_libraries (b2bua davici pthread)
