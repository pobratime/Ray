#include "engine/resources/Mesh.hpp"
#include "engine/resources/Model.hpp"
#include "engine/resources/RayTracingModel.hpp"
#include "engine/resources/Texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/util/Errors.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>
#include <vector>

namespace engine::resources {
void ResourcesController::initialize() {
    load_shaders();
    load_models();
    load_textures();
    load_skyboxes();
}

void ResourcesController::terminate() {
    for (auto &[name, resource]: m_models) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_shaders) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_textures) {
        resource->destroy();
    }
    for (auto &[name, resource]: m_sky_boxes) {
        resource->destroy();
    }
}

void ResourcesController::load_shaders() {
    if (!exists(m_shaders_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the shaders from", m_shaders_path.string());
        return;
    }
    for (const auto &shader_path: std::filesystem::directory_iterator(m_shaders_path)) {
        const auto name = shader_path.path().stem().string();
        shader(name, shader_path);
    }
}

void ResourcesController::load_models() {
    if (!exists(m_models_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the models from", m_models_path.string());
        return;
    }
    const auto &config = util::Configuration::config();
    if (!config.contains("resources") || !config["resources"].contains("models")) {
        std::string msg = "No configuration for models in the config.json, please provide the resources config. See the example in the README.md";
        throw util::EngineError(util::EngineError::Type::ConfigurationError, msg);
    }

    if (config["resources"].value<bool>("build_bvh_on_load", false)) {
        spdlog::info("No models will be loaded in a classic raster way, build_bvh_on_load is set to true");
        for (const auto &model_entry: config["resources"]["models"].items()) {
            rtmodel(model_entry.key());
        }
    } else {
        spdlog::info("No raw geometry will be loaded for model, build_bvh_on_load is set to false");
        for (const auto &model_entry: config["resources"]["models"].items()) {
            model(model_entry.key());
        }
    }
}

void ResourcesController::load_textures() {
    if (!exists(m_textures_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the textures from", m_textures_path.string());
        return;
    }
    for (const auto &texture_entry: std::filesystem::directory_iterator(m_textures_path)) {
        texture(texture_entry.path().stem().string(), texture_entry.path());
    }
}

void ResourcesController::load_skyboxes() {
    if (!exists(m_skyboxes_path)) {
        spdlog::info("[ResourcesController]: no {} found to load the skyboxes from", m_skyboxes_path.string());
        return;
    }
    for (const auto &sky_boxes_entry: std::filesystem::directory_iterator(m_skyboxes_path)) {
        skybox(sky_boxes_entry.path().stem().string(), sky_boxes_entry.path());
    }
}

/**
     * @class AssimpSceneProcessor
     * @brief Processes the meshes in an Assimp scene.
     */
class AssimpSceneProcessor {
public:
    /**
         * @brief Processes the meshes in the scene.
         * @returns The meshes in the scene.
         */


    std::vector<Mesh> process_meshes();
    ResourcesController::RawGeometry process_raw_geometry();

    explicit AssimpSceneProcessor(ResourcesController *resources_controller, const aiScene *scene, std::filesystem::path model_path)
        : m_scene(scene)
        , m_model_path(std::move(model_path))
        , m_resources_controller(resources_controller) {
    }

private:
    void process_node(const aiNode *node, const glm::mat4 &parent_transform);
    void process_mesh(aiMesh *mesh, const glm::mat4 &transform, bool emissive_flag);

    std::vector<uint32_t> extract_indices(const aiMesh *mesh);
    Vertex extract_vertex(const aiMesh *mesh, unsigned int i);

    std::vector<Texture *> process_materials(const aiMaterial *material);

    void process_material_type(std::vector<Texture *> &textures, const aiMaterial *material, aiTextureType type);

    static TextureType assimp_texture_type_to_engine(aiTextureType type);

    std::vector<Mesh> m_meshes;
    ResourcesController::RawGeometry m_rwg;
    const aiScene *m_scene;
    std::filesystem::path m_model_path;
    ResourcesController *m_resources_controller;
    bool m_loading_model_rt = false;
};

const std::vector<RayTracingModel *> &ResourcesController::rtmodels() const {
    return m_rtmodels_ptrs;
}

const std::vector<Texture *> &ResourcesController::rttextures() const {
    return m_textures_ptrs;
}

ResourcesController::ModelLoadParams ResourcesController::resolve_model_params(const std::string &name) {
    auto &config = util::Configuration::config();
    if (!config["resources"]["models"].contains(name)) {
        std::string msg = std::format("No model ({}) specify in config.json. Please add the model to the config.json.", name);
        throw util::EngineError(util::EngineError::Type::ConfigurationError, msg);
    }

    std::filesystem::path model_path = m_models_path / std::filesystem::path(config["resources"]["models"][name]["path"].get<std::string>());
    int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;
    if (config["resources"]["models"][name].value<bool>("flip_uvs", false)) {
        flags |= aiProcess_FlipUVs;
    }
    return {.path = model_path.string(), .assimp_flags = flags};
}

const aiScene *ResourcesController::read_validate_scene(Assimp::Importer &importer, const std::string &name, const ResourcesController::ModelLoadParams &params) {
    const aiScene *scene = importer.ReadFile(params.path, params.assimp_flags);
    spdlog::info("load_model(name={}, path={})", name, params.path);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::string msg = std::format("Assimp error while reading model: {} from path {}.", params.path, name);
        throw util::EngineError(util::EngineError::Type::AssetLoadingError, msg);
    }
    return scene;
}

RayTracingModel *ResourcesController::rtmodel(const std::string &name) {
    auto &result = m_rtmodels[name];
    if (!result) {
        ModelLoadParams params = resolve_model_params(name);
        Assimp::Importer importer;
        const aiScene *scene = read_validate_scene(importer, name, params);
        AssimpSceneProcessor scene_processor(this, scene, params.path);
        RawGeometry rw = scene_processor.process_raw_geometry();
        result = std::make_unique<RayTracingModel>(RayTracingModel(name,
                                                                   std::move(params.path),
                                                                   std::move(rw.vertices),
                                                                   std::move(rw.indices),
                                                                   std::move(rw.texture_indexes),
                                                                   std::move(rw.emissive_local_centroids)));
        m_rtmodels_ptrs.push_back(result.get());
    }
    return result.get();
}

Model *ResourcesController::model(const std::string &name) {
    auto &result = m_models[name];
    if (!result) {
        ModelLoadParams params = resolve_model_params(name);
        Assimp::Importer importer;
        const aiScene *scene = read_validate_scene(importer, name, params);
        AssimpSceneProcessor scene_processor(this, scene, params.path);
        std::vector<Mesh> meshes = scene_processor.process_meshes();
        result = std::make_unique<Model>(Model(std::move(meshes), params.path, name));
    }
    return result.get();
}

Texture *ResourcesController::texture(const std::string &name, const std::filesystem::path &path, TextureType type, bool flip_uvs) {
    auto &result = m_textures[name];
    if (!result) {
        spdlog::info("load_texture(path={})", path.string());
        std::vector<uint8_t> pixels{};
        const auto texture = graphics::OpenGL::generate_texture(path, flip_uvs, pixels);
        result = std::make_unique<Texture>(Texture(texture, type, path, path.stem()));
        result->m_index = static_cast<uint32_t>(m_textures_ptrs.size());
        m_textures_ptrs.push_back(result.get());
    }
    return result.get();
}

Skybox *ResourcesController::skybox(const std::string &name, const std::filesystem::path &path, bool flip_uvs) {
    auto &result = m_sky_boxes[name];
    if (!result) {
        spdlog::info("load_skybox(path={})", path.string());
        auto skybox = graphics::OpenGL::init_skybox_cube();
        auto textures = graphics::OpenGL::load_skybox_textures(path, flip_uvs);
        result = std::make_unique<Skybox>(Skybox(skybox, textures, path, name));
    }
    return result.get();
}

Shader *ResourcesController::shader(const std::string &name, const std::filesystem::path &path) {
    auto &result = m_shaders[name];
    if (!result) {
        spdlog::info("load_shader(path={})", path.string());
        result = std::make_unique<Shader>(ShaderCompiler::compile_from_file(name, path));
    }
    return result.get();
}

ResourcesController::RawGeometry AssimpSceneProcessor::process_raw_geometry() {
    m_rwg.indices.clear();
    m_rwg.vertices.clear();
    m_rwg.texture_indexes.clear();
    m_loading_model_rt = true;
    process_node(m_scene->mRootNode, glm::mat4(1.0f));
    return std::move(m_rwg);
}

std::vector<Mesh> AssimpSceneProcessor::process_meshes() {
    m_meshes.clear();
    process_node(m_scene->mRootNode, glm::mat4(1.0f));
    spdlog::info("scene name -> {}", m_scene->mName.C_Str());
    return std::move(m_meshes);
}

static glm::mat4 ai_matrix_to_glm(const aiMatrix4x4 &m) {
    return glm::mat4(
            m.a1, m.b1, m.c1, m.d1,
            m.a2, m.b2, m.c2, m.d2,
            m.a3, m.b3, m.c3, m.d3,
            m.a4, m.b4, m.c4, m.d4);
}

// FIXED SUB-MODEL LOADING AND APPLIED TRANSFORMATIONS TO SUBMODELS
void AssimpSceneProcessor::process_node(const aiNode *node, const glm::mat4 &parent_transform) {
    glm::mat4 node_transform = ai_matrix_to_glm(node->mTransformation);
    glm::mat4 accumulated_transform = parent_transform * node_transform;

    spdlog::info("model / submodel name -> {}", node->mName.C_Str());
    const std::string model_name = node->mName.C_Str();
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        auto mesh = m_scene->mMeshes[node->mMeshes[i]];
        spdlog::info("mesh name -> {}", mesh->mName.C_Str());
        process_mesh(mesh, accumulated_transform, model_name.contains("emissive"));
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        process_node(node->mChildren[i], accumulated_transform);
    }
}

// FIXED sub-mesh loading
void AssimpSceneProcessor::process_mesh(aiMesh *mesh, const glm::mat4 &transform, bool emissive_flag) {
    std::string mesh_name = mesh->mName.C_Str();
    if (mesh_name.contains("emissive")) {
        emissive_flag = true;
    }
    std::vector<Vertex> vertices;
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(transform)));
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex v = extract_vertex(mesh, i);
        v.Position = glm::vec3(transform * glm::vec4(v.Position, 1.0f));
        v.Normal = glm::normalize(normal_matrix * v.Normal);
        if (mesh->mTextureCoords[0]) {
            v.Tangent = glm::normalize(normal_matrix * v.Tangent);
            v.Bitangent = glm::normalize(normal_matrix * v.Bitangent);
        }
        vertices.push_back(v);
    }
    const std::vector<uint32_t> indices = extract_indices(mesh);

    const auto material = m_scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture *> textures = process_materials(material);

    if (m_loading_model_rt) {
        // ovo promeniti koristiti materialId kao sto je marko preporucio a unutar materialId staviti indekse ovih stvar
        glm::vec4 tex_indices_a{-1.0f};
        glm::vec4 tex_indices_b{-1.0f};
        for (const auto &tex: textures) {
            if (!tex) continue;
            switch (tex->type()) {
                case TextureType::Diffuse: tex_indices_a.x = static_cast<float>(tex->index()); break;
                case TextureType::Specular: tex_indices_a.y = static_cast<float>(tex->index()); break;
                case TextureType::Normal: tex_indices_a.z = static_cast<float>(tex->index()); break;
                case TextureType::Height: tex_indices_a.w = static_cast<float>(tex->index()); break;
                case TextureType::Emissive: tex_indices_b.x = static_cast<float>(tex->index()); break;
                case TextureType::Metalness: tex_indices_b.y = static_cast<float>(tex->index()); break;
                case TextureType::DiffuseRoughness: tex_indices_b.z = static_cast<float>(tex->index()); break;
                case TextureType::AmbientOcclusion: tex_indices_b.w = static_cast<float>(tex->index()); break;
                default: break;
            }
        }
        const size_t num_triangles = indices.size() / 3;
        for (size_t i = 0; i < num_triangles; ++i) {
            m_rwg.texture_indexes.push_back(tex_indices_a);
            m_rwg.texture_indexes.push_back(tex_indices_b);
        }
        const auto base_index = static_cast<uint32_t>(m_rwg.vertices.size());
        for (const uint32_t idx: indices) {
            m_rwg.indices.push_back(base_index + idx);
        }
        if (emissive_flag) {
            glm::vec3 sum(0.0f);
            for (const auto &v: vertices) sum += v.Position;
            glm::vec3 local_centroid = sum / static_cast<float>(vertices.size());
            m_rwg.emissive_local_centroids.push_back(local_centroid);
            spdlog::info("EMISSIVE YAY");
        }
        m_rwg.vertices.insert(m_rwg.vertices.end(), vertices.begin(), vertices.end());
    } else {
        m_meshes.emplace_back(Mesh(vertices, indices, std::move(textures)));
    }
}

Vertex AssimpSceneProcessor::extract_vertex(const aiMesh *mesh, unsigned int i) {
    Vertex vertex{};
    if (mesh->HasPositions()) {
        vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
    }
    if (mesh->HasNormals()) {
        vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
    }
    if (mesh->mTextureCoords[0]) {
        vertex.TexCoords = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
        vertex.Bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
    }
    return vertex;
}

std::vector<uint32_t> AssimpSceneProcessor::extract_indices(const aiMesh *mesh) {
    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }
    return indices;
}

std::vector<Texture *> AssimpSceneProcessor::process_materials(const aiMaterial *material) {
    std::vector<Texture *> textures;
    auto ai_texture_types = {
            aiTextureType_DIFFUSE,
            aiTextureType_SPECULAR,
            aiTextureType_NORMALS,
            aiTextureType_HEIGHT,
            aiTextureType_EMISSIVE,
            aiTextureType_METALNESS,
            aiTextureType_DIFFUSE_ROUGHNESS,
            aiTextureType_AMBIENT_OCCLUSION};

    for (auto ai_texture_type: ai_texture_types) {
        process_material_type(textures, material, ai_texture_type);
    }
    return textures;
}

void AssimpSceneProcessor::process_material_type(std::vector<Texture *> &textures, const aiMaterial *material, aiTextureType type) {
    auto material_count = material->GetTextureCount(type);
    for (uint32_t i = 0; i < material_count; ++i) {
        aiString ai_texture_path_string;
        material->GetTexture(type, i, &ai_texture_path_string);
        std::filesystem::path texture_path = m_model_path.parent_path() / ai_texture_path_string.C_Str();
        Texture *texture = m_resources_controller->texture(texture_path.string(), texture_path, assimp_texture_type_to_engine(type));
        textures.emplace_back(texture);
    }
}

TextureType AssimpSceneProcessor::assimp_texture_type_to_engine(aiTextureType type) {
    switch (type) {
        case aiTextureType_DIFFUSE: return TextureType::Diffuse;
        case aiTextureType_SPECULAR: return TextureType::Specular;
        case aiTextureType_NORMALS: return TextureType::Normal;
        case aiTextureType_HEIGHT: return TextureType::Height;
        case aiTextureType_EMISSIVE: return TextureType::Emissive;
        case aiTextureType_METALNESS: return TextureType::Metalness;
        case aiTextureType_DIFFUSE_ROUGHNESS: return TextureType::DiffuseRoughness;
        case aiTextureType_AMBIENT_OCCLUSION: return TextureType::AmbientOcclusion;
        default: RG_SHOULD_NOT_REACH_HERE("Engine currently doesn't support the aiTextureType: {}", static_cast<int>(type));
    }
}
}// namespace engine::resources
