project(gcontactbash)

add_executable(${PROJECT_NAME} main.cpp GcontactCommands.cpp GcontactCommands.h)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    googleQt
)

