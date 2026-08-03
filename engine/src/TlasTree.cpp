#include "engine/util/TlasTree.hpp"
#include "engine/resources/RayTracingModel.hpp"
#include "engine/util/BlasTree.hpp"

namespace engine::util::ds {

TlasTree::TlasTree(std::vector<resources::RayTracingModel *> &rt_models) {
}

uint32_t TlasTree::build_recursive(std::vector<InstanceBounds> &bounds, uint32_t start, uint32_t end) {
}

}// namespace engine::util::ds
