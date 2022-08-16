project(gmailbash)

add_executable(${PROJECT_NAME} main.cpp GmailCommands.cpp GmailCommands.h)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    googleQt
)

