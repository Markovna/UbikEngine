#pragma once

#include <fstream>
#include "assets.h"

namespace assets::compiler {

template<class T>
bool compile(std::ifstream& stream, const asset& meta, std::ostream&);


}


