cmake_minimum_required(VERSION 3.0)

set(project "dirlist")
set(srcfiles
    "main.cpp"
)

project(${project})
add_executable(${project}
    "main.cpp"
)
target_link_libraries(${project}
    "shlwapi"
)