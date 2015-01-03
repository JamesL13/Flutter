
if (MHD_LIBRARIES AND MHD_INCLUDE_DIRS)
  # in cache already
  set(MHD_FOUND TRUE)
  message(STATUS "Found libmicrohttpd, wunderbar!")
  message(STATUS "    mhd libs at ${MHD_LIBRARIES}")
  message(STATUS "    mhd include dirs at ${MHD_INCLUDE_DIR}")
else (MHD_LIBRARIES AND MHD_INCLUDE_DIRS)
  set(MHD_DEFINITIONS "")

  find_path(MHD_INCLUDE_DIR
    NAMES
      microhttpd.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(MHD_LIBRARY
    NAMES
      microhttpd
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (MHD_LIBRARY)
    set(MHD_FOUND TRUE)
  endif (MHD_LIBRARY)

  set(MHD_INCLUDE_DIRS
    ${MHD_INCLUDE_DIR}
  )

  if (MHD_FOUND)
    set(MHD_LIBRARIES
      ${MHD_LIBRARIES}
      ${MHD_LIBRARY}
    )
  endif (MHD_FOUND)

  if (MHD_INCLUDE_DIRS AND MHD_LIBRARIES)
    set(MHD_FOUND TRUE)
    message(STATUS "Found libmicrohttpd, wunderbar!")
    message(STATUS "    mhd libs at ${MHD_LIBRARIES}")
    message(STATUS "    mhd include dirs at ${MHD_INCLUDE_DIR}")
  endif (MHD_INCLUDE_DIRS AND MHD_LIBRARIES)

  if (MHD_FIND_REQUIRED AND NOT MHD_FOUND)
    message(FATAL_ERROR "libmicrohttpd required, go here: http://www.gnu.org/software/libmicrohttpd/")
  endif (MHD_FIND_REQUIRED AND NOT MHD_FOUND)

  # show the MHD_INCLUDE_DIRS and MHD_LIBRARIES variables only in the advanced view
  mark_as_advanced(MHD_INCLUDE_DIRS MHD_LIBRARIES)

endif (MHD_LIBRARIES AND MHD_INCLUDE_DIRS)

