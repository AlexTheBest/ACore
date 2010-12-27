# Copyright (C) 2005-2010 Trinity <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

if( WITH_SQL )
<<<<<<< HEAD:sql/CMakeLists.txt
  add_subdirectory(TriniChat2)
  add_subdirectory(updates)
  add_subdirectory(scripts)
  add_subdirectory(base)
  add_subdirectory(create)
=======
  install(
    DIRECTORY
      updates
      scripts
      base
      create
    DESTINATION
      shared/trinity/sql
  )
>>>>>>> 961c6678d57779cbc4caa3ea71ad0f4f1143fa20:sql/CMakeLists.txt
endif()
