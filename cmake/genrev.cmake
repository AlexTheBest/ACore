# Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

execute_process(
  COMMAND git describe --match init --dirty=+ --abbrev=12
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE rev_info
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

# Last minute check - ensure that we have a proper revision
# If everything above fails (means the user has erased the git revision control directory or removed the origin/HEAD tag)
if(NOT rev_info)
  message("")
  message(STATUS "WARNING - No revision-information found")

  # Ok, since we have no valid ways of finding/setting the revision, let's force some defaults
  set(rev_hash_str "Archive")
  set(rev_hash "0")
  set(rev_id_str "0")
  set(rev_id "0")
else()
  # Extract revision and hash from git
  string(REGEX REPLACE init-|[0-9]+-g "" rev_hash_str ${rev_info})
  string(REGEX REPLACE [+]+ "" rev_hash ${rev_hash_str})
  string(REGEX REPLACE init-|-g[^+]+ "" rev_id_str ${rev_info})
  string(REGEX REPLACE [+]+ "" rev_id ${rev_id_str})
endif()

# Its not set during initial run
if(NOT BUILDDIR)
  set(BUILDDIR ${CMAKE_BINARY_DIR})
endif()

# Create the actual revision.h file from the above params
if(NOT "${rev_id_cached}" MATCHES "${rev_id_str}")
  configure_file(
    "${CMAKE_SOURCE_DIR}/revision.h.in.cmake"
    "${BUILDDIR}/revision.h"
    @ONLY
  )
  set(rev_id_cached "${rev_id_str}" CACHE INTERNAL "Cached revision ID")
endif()
