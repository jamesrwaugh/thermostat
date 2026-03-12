idf_component_register()

find_package(etl)

target_link_libraries(${COMPONENT_LIB} INTERFACE etl::etl)