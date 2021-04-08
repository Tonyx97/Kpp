#pragma once

#include <string>
#include <algorithm>
#include <random>
#include <vector>
#include <queue>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <chrono>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <list>
#include <any>
#include <stack>
#include <optional>
#include <functional>
#include <future>
#include <ranges>
#include <span>

#include "debug.h"
#include "rtti.h"
#include "utils.h"
#include "err_handler.h"

#ifdef _DEBUG
#pragma comment(lib, "opencv_world451d.lib")
#else
#pragma comment(lib, "opencv_world451.lib")
#endif

#include <opencv2/opencv_modules.hpp>
#include <opencv2/opencv.hpp>

#include "gv.h"