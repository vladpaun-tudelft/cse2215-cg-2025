// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#if defined(WIN32)
#define NOMINMAX
#include <Windows.h>
#endif
#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
DISABLE_WARNINGS_POP()
#include "vector_matrix_tests.h"
#include "nGonOrder_tests.h"
#include "solid_tests.h"
#include "nGonArea_tests.h"
#include "linear.h"
