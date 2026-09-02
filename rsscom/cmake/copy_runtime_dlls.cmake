# A function that copies all runtime dependencies fo TGT to DESTINATION
# This function looks at all linked libraries of TGT for dependents

function(COPY_RUNTIME_DLLS TGT DESTINATION)
    get_target_property(LINK_LIBRARIES ${TGT} LINK_LIBRARIES)
    # message("Searching for DLL only dependencies...")
    foreach(LINK_LIBRARY IN ITEMS ${LINK_LIBRARIES})

        get_target_property(LIBRARY_TYPE ${LINK_LIBRARY} TYPE)
        if(NOT LIBRARY_TYPE STREQUAL INTERFACE_LIBRARY)
            # message("${LINK_LIBRARY} is ${LIBRARY_TYPE}... Ignoring...")
            continue()
        endif()

        # message("Probable match with ${LINK_LIBRARY}.")

        get_target_property(DLL_LOCATION         ${LINK_LIBRARY} IMPORTED_LOCATION)
        get_target_property(DLL_LOCATION_RELEASE ${LINK_LIBRARY} IMPORTED_LOCATION_RELEASE)
        get_target_property(DLL_LOCATION_DEBUG   ${LINK_LIBRARY} IMPORTED_LOCATION_DEBUG)

        # message(" DLL Location (Default): ${DLL_LOCATION}")
        # message(" DLL Location (Release): ${DLL_LOCATION_RELEASE}")
        # message(" DLL Location (Debug)  : ${DLL_LOCATION_DEBUG}")


        if(DLL_LOCATION)
            # Assume that this is a DLL only library (without LIB)
            # And both Release and Debug configurations use the same version
            # message("${LINK_LIBRARY} - found default DLL - using as Release and Debug")
            list(APPEND RUNTIME_DLLS_RELEASE ${DLL_LOCATION})
            list(APPEND RUNTIME_DLLS_DEBUG   ${DLL_LOCATION})
        elseif(NOT DLL_LOCATION AND DLL_LOCATION_RELEASE AND DLL_LOCATION_DEBUG)
            # There are separate DLLs for each configuration (Release and Debug)
            # message("${LINK_LIBRARY} - found specific Release and Debug DLLs")
            list(APPEND RUNTIME_DLLS_RELEASE ${DLL_LOCATION_RELEASE})
            list(APPEND RUNTIME_DLLS_DEBUG   ${DLL_LOCATION_DEBUG})
        else()
            # message("${LINK_LIBRARY} - There are no DLLs specified. The library is probably header-only INTERFACE library")
        endif()

    endforeach()

    # message("${RUNTIME_DLLS_RELEASE}")
    # message("${RUNTIME_DLLS_DEBUG}")


    add_custom_command(TARGET ${TGT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    -t ${DESTINATION}
    "$<TARGET_RUNTIME_DLLS:${TGT}>"                 
    "$<$<CONFIG:Release>:${RUNTIME_DLLS_RELEASE}>"  
    "$<$<CONFIG:Debug>:${RUNTIME_DLLS_DEBUG}>"
    COMMAND_EXPAND_LISTS
    )

endfunction()