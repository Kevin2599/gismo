### CMakeLists.txt ---
## 
## Author: Angelos Mantzaflaris 
## Copyright (C) 2016 - RICAM-Linz.
######################################################################

## Unum extension
project(gsUnumExtension)

# Apply same configuration as G+Smo
include(gsConfig)

gismo_fetch_directory(Unum
    GIT_REPOSITORY https://github.com/stillwater-sc/universal.git
    DESTINATION    external
)

set(UNUM_INCLUDE_DIR ${gismo_SOURCE_DIR}/external/Unum/posit CACHE INTERNAL "")
include_directories(${UNUM_INCLUDE_DIR})

install(DIRECTORY ${UNUM_INCLUDE_DIR}
        DESTINATION include/posit/ OPTIONAL)
