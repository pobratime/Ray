#include "engine/util/TlasTree.hpp"
#include "engine/resources/RayTracingModel.hpp"
#include "engine/util/BlasTree.hpp"
#include "glm/common.hpp"
#include <cassert>
#include <cstdint>

namespace engine::util::ds {
TlasTree::TlasTree(std::vector<resources::RayTracingModel *> &rt_models) {
    if (rt_models.empty()) return;
    std::vector<InstanceBounds> bounds{};
    bounds.reserve(rt_models.size());
    for (uint32_t i = 0; i < static_cast<uint32_t>(rt_models.size()); i++) {
        BlasTree::BlasNode node = rt_models[i]->m_blas.nodes()[0];
        glm::mat4 local_to_world = rt_models[i]->get_local_to_world();
        glm::mat4 world_to_local = rt_models[i]->get_world_to_local();

        glm::vec3 local_min = node.min_bound;
        glm::vec3 local_max = node.max_bound;

        auto world_min = glm::vec3(FLT_MAX);
        auto world_max = glm::vec3(-FLT_MAX);

        for (int c = 0; c < 8; ++c) {
            auto corner = glm::vec3(
                    (c & 1) ? local_max.x : local_min.x,
                    (c & 2) ? local_max.y : local_min.y,
                    (c & 4) ? local_max.z : local_min.z);

            auto world_corner = glm::vec3(local_to_world * glm::vec4(corner, 1.0f));
            world_min = glm::min(world_min, world_corner);
            world_max = glm::max(world_max, world_corner);
        }

        bounds.emplace_back(world_min, world_max,
                            (world_min + world_max) * 0.5f, i);

        m_instances.emplace_back(
                world_to_local, local_to_world,
                rt_models[i]->m_blas_root_offset,
                0, 0, 0);
    }

    build_recursive(bounds, 0, static_cast<uint32_t>(rt_models.size()));
}

uint32_t TlasTree::build_recursive(std::vector<InstanceBounds> &bounds,
                                   uint32_t start, uint32_t end) {
    const auto node_index = static_cast<uint32_t>(m_nodes.size());
    TlasNode node{};
    const InstanceBounds b = compute_bounds(bounds, start, end);
    node.min_bound = b.min;
    node.max_bound = b.max;
    m_nodes.push_back(node);
    if (end - start <= MAX_LEAF_INSTANCES) {
        m_nodes[node_index].first_instance = bounds[start].instance_index;
        m_nodes[node_index].instance_count = end - start;
        return node_index;
    }


    glm::vec3 diff = m_nodes[node_index].max_bound - m_nodes[node_index].min_bound;
    int axis = 0;                     // x
    if (diff.y > diff.x) axis = 1;    // y
    if (diff.z > diff[axis]) axis = 2;//z
    if (diff.x == 0.0f && diff.y == 0.0f && diff.z == 0.0f) {
        m_nodes[node_index].first_instance = start;
        m_nodes[node_index].instance_count = end - start;
        return node_index;
    }

    uint32_t mid = (start + end) / 2;
    std::nth_element(bounds.begin() + start,
                     bounds.begin() + mid,
                     bounds.begin() + end,
                     [axis](const InstanceBounds &a, const InstanceBounds &b) {
                         return a.centroid[axis] < b.centroid[axis];
                     });

    const uint32_t left = build_recursive(bounds, start, mid);
    const uint32_t right = build_recursive(bounds, mid, end);
    m_nodes[node_index].left_child = left;
    m_nodes[node_index].right_child = right;
    m_nodes[node_index].instance_count = 0;

    return node_index;
}

TlasTree::InstanceBounds TlasTree::compute_bounds(std::vector<InstanceBounds> &bounds,
                                                  uint32_t start, uint32_t end) {
    InstanceBounds bound{};
    bound.min = bounds[start].min;
    bound.max = bounds[start].max;
    for (uint32_t i = start + 1; i < end; i++) {
        bound.min = glm::min(bound.min, bounds[i].min);
        bound.max = glm::max(bound.max, bounds[i].max);
    }
    return bound;
}
}// namespace engine::util::ds
