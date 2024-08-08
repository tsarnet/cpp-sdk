add_executable (tsar_example_app)
set_target_properties (tsar_example_app PROPERTIES OUTPUT_NAME "app")
target_sources (tsar_example_app PRIVATE app.cpp)
target_link_libraries (tsar_example_app PRIVATE tsar)