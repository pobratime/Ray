#pragma once

#include "engine/resources/Mesh.hpp"
#include "glm/fwd.hpp"
#include <engine/util/BVHTree.hpp>
#include <string>


namespace engine::resources {
class RawGeometry {
public:
    RawGeometry(std::vector<Vertex> vertices,
                std::vector<uint32_t> indices)
        : m_vertices(vertices)
        , m_indices(indices) {};

private:
    std::vector<Vertex> m_vertices;
    std::vector<glm::uint32_t> m_indices;
};

class RayTracingModel {
    friend class ResourcesController;

public:
    std::filesystem::path m_path;
    std::string m_name;
    util::ds::BVHTree m_bvh;

private:
    RayTracingModel(std::filesystem::path path,
                    std::string name, util::ds::BVHTree bvh)
        : m_path(path)
        , m_name(name)
        , m_bvh(bvh) {};
};

}// namespace engine::resources
