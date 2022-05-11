#include "mesh.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

void Mesh::load(const char* path) {
  const aiScene* scene = aiImportFile(path, aiProcess_Triangulate);
  if (!scene || !scene->HasMeshes()) {
    std::cerr << "Unable to load: " << path << '\n';
    exit(EXIT_FAILURE);
  }

  const aiMesh* mesh = scene->mMeshes[0];
  std::vector<Vertex> vertices(mesh->mNumVertices);
  for (auto i = 0; i < mesh->mNumVertices; ++i) {
    const aiFace& face = mesh->mFaces[i];
    const aiVector3D v = mesh->mVertices[i];
    const aiVector3D n = mesh->mNormals[i];
    const aiVector3D uv = mesh->mTextureCoords[0][i];
    vertices[i] = {
        {v.x, v.y, v.z},
        {n.x, n.y, n.z},
        {uv.x, uv.y},
    };
  }

  std::vector<uint32_t> indices(mesh->mNumFaces * 3);
  for (auto i = 0; i < mesh->mNumFaces; ++i) {
    for (unsigned j = 0; j != 3; j++) {
      indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
  }
  aiReleaseImport(scene);

  const size_t size_indices = sizeof(uint32_t) * indices.size();
  const size_t size_vertices = sizeof(Vertex) * vertices.size();
  index_count_ = indices.size();

  vao_.bind();
  index_buffer_.set_data(size_indices, indices.data());
  vao_.set_element_buffer(index_buffer_);

  vertex_buffer_.set_data(size_vertices, vertices.data());
  vao_.set_vertex_buffer(0, vertex_buffer_, 0, sizeof(Vertex));
  vao_.set_attribute_enabled(0, true);
  vao_.set_attribute_binding(0, 0);
  vao_.set_attribute_format(0, 3, GL_FLOAT, GL_FALSE, 0);
  vao_.set_attribute_enabled(1, true);
  vao_.set_attribute_binding(1, 0);
  vao_.set_attribute_format(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  vao_.set_attribute_enabled(2, true);
  vao_.set_attribute_binding(2, 0);
  vao_.set_attribute_format(2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
}

void Mesh::bind() {
  vao_.bind();
}

uint32_t Mesh::getIndexCount() const {
  return index_count_;
}
