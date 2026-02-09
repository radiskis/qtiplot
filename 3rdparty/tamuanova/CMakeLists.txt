# 3rdparty/tamuanova/CMakeLists.txt

add_library(tamuanova STATIC
    anova_1.c
    anova_2.c
)

target_include_directories(tamuanova PUBLIC . ${PROJECT_SOURCE_DIR}/3rdparty/gsl)
target_link_libraries(tamuanova PRIVATE gsl)
