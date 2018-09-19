project(utoxx_av LANGUAGES C)

add_library(utoxx_av STATIC
    utox_av.c
    audio.c
    video.c
    )

if(WIN32)
    target_link_libraries(utoxx_av OpenAL32) # Windows needs to be linked against OpenAL32
else()
    # Both openal and opus are required for the static Linux builds on Jenkins.
    find_package(libopus REQUIRED)
    include_directories(${LIBOPUS_INCLUDE_DIRS})
    target_link_libraries(utoxx_av ${LIBOPUS_LIBRARIES})
    target_link_libraries(utoxx_av openal)
endif()

if(ENABLE_FILTERAUDIO)
    find_package(libfilteraudio REQUIRED)
    include_directories(${LIBFILTERAUDIO_INCLUDE_DIRS})
    target_link_libraries(utoxx_av ${LIBFILTERAUDIO_LIBRARIES})
endif()
