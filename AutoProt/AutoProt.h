#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "API/ARK/Ark.h"
#include "../../json.hpp"

namespace AutoProt {
	extern nlohmann::json config;
}

void load_config();
