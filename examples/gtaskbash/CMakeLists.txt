project(gtaskbash)

add_executable(${PROJECT_NAME} main.cpp GtaskCommands.cpp GtaskCommands.h)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    googleQt
)

