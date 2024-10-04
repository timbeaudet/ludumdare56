///
/// @file
/// @details Provides an interface for creating a mesh from data. This will probably get included in ICE someday.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef ludumdare56_headless_build

#include "mesh_creator.hpp"

//--------------------------------------------------------------------------------------------------------------------//

Render::MeshCreator::MeshCreator(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

Render::MeshCreator::~MeshCreator(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void Render::MeshCreator::ClearVertices(void)
{
	mVertices.clear();
}

//--------------------------------------------------------------------------------------------------------------------//

void Render::MeshCreator::AddVertex(const tbMath::Vector3& position, const tbMath::Vector3& normal, const tbGraphics::Color& color, const tbMath::Vector2& uv)
{
	iceGraphics::MeshVertex vertex;
	vertex.mPosition = position;
	vertex.mNormal = normal;
	vertex.mColor = color.GetColorARGB();
	vertex.mTextureUV = uv;
	mVertices.push_back(vertex);
}

//--------------------------------------------------------------------------------------------------------------------//

std::vector<tbCore::uint32> Render::MeshCreator::CreateIndices(void) const
{
	std::vector<tbCore::uint32> indices;

	tb_error_if(tbImplementation::Renderer::kTriangleStrip != mPrimitiveType, "Only TriangleStrips are supported at this time...");

	bool flip = true;
	for (iceCore::VertexIndex vertexIndex = 0; vertexIndex < mVertices.size() - 2; ++vertexIndex)
	{
		indices.push_back(vertexIndex);
		if (flip)
		{
			indices.push_back(vertexIndex + 1);
			indices.push_back(vertexIndex + 2);
		}
		else
		{
			indices.push_back(vertexIndex + 2);
			indices.push_back(vertexIndex + 1);
		}
		flip = !flip;
	}

	return indices;
}

//--------------------------------------------------------------------------------------------------------------------//

iceGraphics::MeshHandle Render::MeshCreator::GetMeshHandle(void)
{
	tbCore::uint8 meshFlags = iceCore::MeshFlags::kPosition | iceCore::MeshFlags::kNormal |
		iceCore::MeshFlags::kTexture0 | iceCore::MeshFlags::kDiffuse;

	return iceGraphics::theMeshManager.CreateMeshFromData(mVertices, CreateIndices(), meshFlags);
}

#endif /* ludumdare56_headless_build */

//--------------------------------------------------------------------------------------------------------------------//
