cmake_minimum_required(VERSION 3.22.1)

project("skiaseekbar")

add_library(skiaseekbar SHARED
        main.cpp
        AndroidDebug/AndroidOut.cpp
        Renderer.cpp
        Shader/Shader.cpp
        Texture/TextureAsset.cpp
        Common/Utility.cpp
        UI/SeekbarLayout.cpp
        UI/SeekbarView.cpp
        Input/SeekbarController.cpp
        Render/SkiaRasterTexture.cpp
        Render/FullscreenTexturedQuad.cpp
        Animation/CircleProgress.cpp
)


# ---- Skia (prebuilt) ----
set(SKIA_ROOT ${CMAKE_CURRENT_LIST_DIR}/third_party/skia)
set(SKIA_INCLUDE ${SKIA_ROOT}/include)
set(SKIA_GEN     ${SKIA_ROOT}/gen)
set(SKIA_LIB_DIR ${SKIA_ROOT}/lib/${ANDROID_ABI})

add_library(skia STATIC IMPORTED)
set_target_properties(skia PROPERTIES
        IMPORTED_LOCATION ${SKIA_LIB_DIR}/libskia.a
)

target_include_directories(skiaseekbar PRIVATE
        ${SKIA_GEN}
        ${SKIA_INCLUDE}
)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

# Forces the linker to keep the JNI entry point for GameActivity
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

target_link_libraries(skiaseekbar
        game-activity::game-activity_static
        skia
        EGL
        GLESv3
        jnigraphics
        android
        log)
