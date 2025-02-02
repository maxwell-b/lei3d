#include "Model.hpp"

#include "logging/LogGLM.hpp"

namespace lei3d
{

	Model::Model(const std::string& modelPath)
	{
		loadModel(modelPath);
	}

	Model::~Model()
	{
		for (btTriangleMesh* mesh : m_BTMeshes)
		{
			delete mesh;
		}
	}

	void Model::Draw(Shader& shader, RenderFlag flags, uint32_t bindLocation)
	{
		for (unsigned int i = 0; i < this->m_Meshes.size(); i++)
		{
			m_Meshes[i].Draw(shader, flags, bindLocation);
		}
	}

	void Model::loadModel(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene*	 scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::string errorString = importer.GetErrorString();
			LEI_WARN("ERROR::ASSIMP::" + errorString);
			return;
		}
		m_Directory = path.substr(0, path.find_last_of('/'));

		loadMaterials(scene);
		processNode(scene->mRootNode, scene);
	}

	void Model::processNode(aiNode* node, const aiScene* scene)
	{
		// process node's meshes
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(processMesh(mesh, scene));
		}

		// process node children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	/**
	 * Given an mesh and the scene it belongs to, create our own representation of a mesh
	 * object that we can render.
	 *
	 */
	Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex>		  vertices;
		std::vector<unsigned int> indices;

		// process vertices from assimp to our own mesh component
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};

			glm::vec3 pos;
			pos.x = mesh->mVertices[i].x;
			pos.y = mesh->mVertices[i].y;
			pos.z = mesh->mVertices[i].z;
			vertex.Position = pos;

			if (mesh->HasNormals())
			{
				glm::vec3 normal;
				normal.x = mesh->mNormals[i].x;
				normal.y = mesh->mNormals[i].y;
				normal.z = mesh->mNormals[i].z;
				vertex.Normal = normal;
			}

			if (mesh->mTextureCoords[0]) // are there any uv coordinates associated with mesh
			{
				glm::vec2 uv;
				uv.x = mesh->mTextureCoords[0][i].x;
				uv.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = uv;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			if (mesh->mTangents)
			{
				glm::vec3 tangent(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
				vertex.Tangent = tangent;
			}

			vertices.push_back(vertex);
		}

		// we set all of our faces to be triangles, so it's easy to get the indices we need
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// finally load all the materials we want
		Material* material = nullptr;
		if (mesh->mMaterialIndex >= 0)
		{
			material = materials[mesh->mMaterialIndex].get();
		}

		// return a mesh object created from the extracted mesh data
		return { vertices, indices, material };
	}

	// get the materials that we want from the assimp mat and convert it to textures array that is returned
	Texture* Model::loadMaterialTexture(const aiMaterial* mat, aiTextureType type, const std::string& typeName)
	{
		Texture* texture;

		if (mat->GetTextureCount(type) < 1)
		{
			// no textures of this type
			return nullptr;
		}
		if (mat->GetTextureCount(type) > 1)
		{
			std::cout << "Found more than 1 texture for type " << aiTextureTypeToString(type) << " in material " << mat->GetName().C_Str() << "\n";
		}

		// only select the first texture
		aiString str;
		mat->GetTexture(type, 0, &str);

		bool skip = false;
		for (const auto& tex : textures) // for every texture we've loaded so far
		{
			if (std::strcmp(str.C_Str(), tex->path.c_str()) == 0)
			{
				texture = tex.get();
				skip = true;
			}
		}

		if (!skip)
		{
			texture = new Texture();
			texture->id = TextureFromFile(str.C_Str(), m_Directory);
			texture->type = typeName;
			texture->path = str.C_Str();
			textures.emplace_back(texture);
			texture = textures.back().get();
		}

		return texture;
	}

	/**
	 * @brief Creates collision meshes From Model object
	 *
	 * Requires the Model to have loaded it's meshes. Leverages the vertices and indices of each mesh to create a
	 * btTriangleMesh for each Mesh.
	 *
	 * @return std::vector<btTriangleMesh>
	 */
	std::vector<btTriangleMesh*>& Model::GetCollisionMeshes()
	{
		if (m_BTMeshes.empty())
		{
			for (const Mesh& mesh : m_Meshes)
			{
				btTriangleMesh* curCollisionMesh = new btTriangleMesh();

				std::vector<Vertex>		  vertices = mesh.vertices;
				std::vector<unsigned int> indices = mesh.indices;

				for (int i = 0; i < indices.size(); i += 3)
				{
					glm::vec3 vert1 = vertices[indices[i]].Position;
					btVector3 bvert1(vert1.x, vert1.y, vert1.z);
					glm::vec3 vert2 = vertices[indices[i + 1]].Position;
					btVector3 bvert2(vert2.x, vert2.y, vert2.z);
					glm::vec3 vert3 = vertices[indices[i + 2]].Position;
					btVector3 bvert3(vert3.x, vert3.y, vert3.z);

					curCollisionMesh->addTriangle(bvert1, bvert2, bvert3);
				}
				m_BTMeshes.push_back(curCollisionMesh);
			}
		}

		return m_BTMeshes;
	}

	void Model::loadMaterials(const aiScene* scene)
	{
		for (size_t i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial* aimaterial = scene->mMaterials[i];

			Material* newMaterial = new Material();

			aiColor3D color;
			if (aimaterial->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS)
			{
				newMaterial->m_Albedo = glm::vec3(color.r, color.g, color.b);
			}
			if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
			{
				newMaterial->m_Albedo = glm::vec3(color.r, color.g, color.b);
			}

			if (aimaterial->Get(AI_MATKEY_METALLIC_FACTOR, newMaterial->m_Metallic) != AI_SUCCESS)
			{
				aimaterial->Get(AI_MATKEY_SPECULAR_FACTOR, newMaterial->m_Metallic);
			}
			if (aimaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, newMaterial->m_Roughness) != AI_SUCCESS)
			{
				if (aimaterial->Get(AI_MATKEY_GLOSSINESS_FACTOR, newMaterial->m_Roughness) == AI_SUCCESS)
				{
					newMaterial->m_Roughness = 1 - newMaterial->m_Roughness;
				}
			}

			newMaterial->m_AlbedoTexture = loadMaterialTexture(aimaterial, aiTextureType_DIFFUSE, "texture_diffuse");
			newMaterial->m_MetallicTexture = loadMaterialTexture(aimaterial, aiTextureType_METALNESS, "texture_metallic");
			newMaterial->m_RoughnessTexture = loadMaterialTexture(aimaterial, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
			newMaterial->m_AmbientTexture = loadMaterialTexture(aimaterial, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");

			newMaterial->m_NormalMap = loadMaterialTexture(aimaterial, aiTextureType_NORMALS, "texture_normal");
			newMaterial->m_BumpMap = loadMaterialTexture(aimaterial, aiTextureType_HEIGHT, "texture_bump");

			newMaterial->m_UseAlbedoMap = newMaterial->m_AlbedoTexture != nullptr;
			newMaterial->m_UseMetallicMap = newMaterial->m_MetallicTexture != nullptr;
			newMaterial->m_UseRoughnessMap = newMaterial->m_RoughnessTexture != nullptr;
			newMaterial->m_UseAmbientMap = newMaterial->m_AmbientTexture != nullptr;
			newMaterial->m_UseNormalMap = newMaterial->m_NormalMap != nullptr;
			newMaterial->m_UseBumpMap = newMaterial->m_BumpMap != nullptr;

			materials.emplace_back(newMaterial);
		}
	}

	// from https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/model.h
	unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int			   width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			LEI_WARN("Texture failed to load at path: " + std::string(path));
			stbi_image_free(data);
		}

		return textureID;
	}

} // namespace lei3d