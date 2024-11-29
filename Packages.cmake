include(CPM.cmake)

CPMAddPackage(
  NAME fmt
  GITHUB_REPOSITORY fmtlib/fmt
  GIT_TAG 11.0.2
  OPTIONS
    "FMT_DOC OFF"
    "FMT_TEST OFF"
)

CPMAddPackage(
        NAME raylib
        GITHUB_REPOSITORY raysan5/raylib
        GIT_TAG 5.5
        OPTIONS
        "BUILD_EXAMPLES OFF"
        "BUILD_GAMES OFF"
        "BUILD_TEST OFF"
)
