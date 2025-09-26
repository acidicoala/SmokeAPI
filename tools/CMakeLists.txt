cmake_minimum_required(VERSION 3.24)

project(SmokeAPITools LANGUAGES CXX)

### Steamworks Downloader executable

add_executable(steamworks_downloader src/steamworks_downloader.cpp)
target_link_libraries(steamworks_downloader PRIVATE KoalaBoxTools)

### Steamworks Parser executable

add_executable(steamworks_parser src/steamworks_parser.cpp)
target_link_libraries(steamworks_parser PRIVATE KoalaBoxTools)
