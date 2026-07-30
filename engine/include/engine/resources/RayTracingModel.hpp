#pragma once

#include <engine/util/BVHTree.hpp>
#include <string>


namespace engine::resources {
class RawGeometry {
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
