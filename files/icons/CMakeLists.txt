if(WIN32)
  IF( MINGW )
    set(PANDORICA_ICON_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/pandorica_icon.o)
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND( OUTPUT ${PANDORICA_ICON_OBJECT}
                        COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/pandorica.rc 
                             -o ${PANDORICA_ICON_OBJECT} )
  ELSE( MINGW )
    set(PANDORICA_ICON_OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/pandorica.rc)
  ENDIF( MINGW )
endif(WIN32)
