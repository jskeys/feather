execute_process(OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND bash -c "git describe --abbrev=7 --dirty --always --tags" OUTPUT_VARIABLE GIT_VERSION)
idf_component_register(SRCS "main.c" "app.c" "temp_sensor.c"
                       INCLUDE_DIRS "include")
add_compile_definitions(APP_HASH="${GIT_VERSION}")
