#include "ecs.h"

namespace ecs {

uint32_t details::next_index() {
    static size_t value{};
    return value++;
}

}