#pragma once

#define REAL_CONCAT(x, y) x ## y
#define CONCAT(x, y) REAL_CONCAT(x, y)
#define EMPTY
#define DEFER(x) x EMPTY
#define EVAL(x) x
