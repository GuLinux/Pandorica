include(FindPkgConfig)
pkg_check_modules(GraphicsMagick GraphicsMagick++ REQUIRED)
message("Magick found: ${GraphicsMagick_FOUND}, ${GraphicsMagick_CFLAGS}, ${GraphicsMagick_LIBRARIES}")
add_definitions(${GraphicsMagick_CFLAGS})