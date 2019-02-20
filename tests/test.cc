#include "../matrix.hpp"

SUPPRESS_WARNINGS
#define CATCH_CONFIG_MAIN
#include "./catch.hpp"

#include "./cases/inverse.hpp"
#include "./cases/misc_range.hpp"
#include "./cases/ones.hpp"
#include "./cases/operator_equal.hpp"
#include "./cases/size.hpp"
#include "./cases/sin.hpp"
#include "./cases/proj.hpp"
#include "./cases/norm.hpp"
#include "./cases/view_bracket.hpp"
#include "./cases/zeros.hpp"
#include "./cases/minmax.hpp"

RESTORE_WARNINGS

