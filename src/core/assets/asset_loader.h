#pragma once

#include <memory>
#include <istream>

namespace assets::loader {

template<class T>
std::unique_ptr<T> load(std::istream&);

}