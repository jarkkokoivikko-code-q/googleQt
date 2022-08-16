project(gdrivebash)

add_executable(${PROJECT_NAME} main.cpp GdriveCommands.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    googleQt
)

