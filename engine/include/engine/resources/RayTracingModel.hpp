#pragma once

#include "engine/resources/Mesh.hpp"
#include "glm/fwd.hpp"
#include <cstdint>
#include <engine/util/BlasTree.hpp>
#include <string>


namespace engine::resources {
class RawGeometry {
public:
    RawGeometry(std::vector<Vertex> vertices,
                std::vector<uint32_t> indices)
        : m_vertices(vertices)
        , m_indices(indices) {};

    const std::vector<Vertex> &vertices() const {
        return m_vertices;
    }

    const std::vector<uint32_t> &indices() const {
        return m_indices;
    }


private:
    std::vector<Vertex> m_vertices;
    std::vector<glm::uint32_t> m_indices;
};

class RayTracingModel {
    friend class ResourcesController;

public:
    std::string m_name;
    util::ds::BlasTree m_bvh;
    void bind(const unsigned int bvh, const unsigned int nodes) {
        m_bvh.bind(bvh, nodes);
    }

private:
    RayTracingModel(
            std::string name, util::ds::BlasTree bvh)
        : m_name(std::move(name))
        , m_bvh(std::move(bvh)) {};
};

}// namespace engine::resources
