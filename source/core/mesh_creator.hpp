///
/// @file
/// @details Provides an interface for creating a mesh from data. This will probably get included in ICE someday.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Undefined_MeshCreator_hpp
#define Undefined_MeshCreator_hpp

#ifndef ludumdare56_headless_build

#include <turtle_brains/core/tb_noncopyable.hpp>
#include <turtle_brains/graphics/tb_color.hpp>
#include <turtle_brains/math/tb_vector.hpp>

#include <ice/core/ice_mesh_data.hpp>
#include <ice/core/ice_mesh_manager.hpp>

#include <vector>

namespace Render
{

	///
	///	@details Allow creation of simple and complex shapes by supplying several points that get shaped into triangles.
	///
	class MeshCreator : public tbCore::Noncopyable
	{
	public:

		///
		///	@details Constructs an empty PolygonShape object that contains no vertices.
		///
		MeshCreator(void);

		/////
		/////	@details Constructs a PolygonShape object with the same properties as the other object being copied.
		/////
		//MeshCreator(const MeshCreator& other);

		///
		///	@details Cleans up after a PolygonShape, clearing the vertex container in the process.
		///
		virtual ~MeshCreator(void);


		///
		///	@details Clears all the vertices in the PolygonShape.
		///
		virtual void ClearVertices(void);

		///
		///	@details Adds a vertex to a container of vertices for the PolygonShape. How it will be rendered depends on
		///   how the PolygonShape object is configured.
		///
		virtual void AddVertex(const tbMath::Vector3& position, const tbMath::Vector3& normal = tbMath::Vector3(0.0f, 1.0f, 0.0f),
			const tbGraphics::Color& color = tbGraphics::ColorPalette::White, const tbMath::Vector2& uv = tbMath::Vector2::Zero());

		iceCore::MeshHandle GetMeshHandle(void);

		///
		///	@details Configures the object to treat the vertices as a list of Triangles. In this mode the number of
		///   vertices must be equal-to or greater-than 3, and must also be divisible by 3.
		///
		void SetAsTriangles(void) { mPrimitiveType = tbImplementation::Renderer::kTriangles; }

		///
		///	@details Configures the object to treat the vertices as a fan of Triangles. In this mode the number of
		///   vertices must be equal-to or greater-than 3. The first vertex added will be the center of the fan and used
		///   for each of the triangles.
		///
		void SetAsTriangleFan(void) { mPrimitiveType = tbImplementation::Renderer::kTriangleFan; }

		///
		/// @details Configures the object to treat the vertices as a strip of Triangles. In this mode the number of
		///   vertices must be at least 3. Each vertex after the third will create a triangle using the two vertices
		///   added immediately before it.
		///
		void SetAsTriangleStrip(void) { mPrimitiveType = tbImplementation::Renderer::kTriangleStrip; }

		///
		/// @details Configures the object to treat the vertices as a list of lines. In this mode the number of vertices
		///   must be equal-to or greater-than 2, and must also be divisible by 2. Each pair of vertices added will form
		///   a line segment.
		///
		void SetAsLines(void) { mPrimitiveType = tbImplementation::Renderer::kLines; }

		///
		/// @details Configures the object to treat the vertices as a line strip. In this mode the number of vertices must
		///   be at least 2. Each vertex added after the first will create a line segment to the vertex added previously.
		///
		void SetAsLineStrip(void) { mPrimitiveType = tbImplementation::Renderer::kLineStrip; }

		///
		/// @details Configures the object to treat the vertices as a line loop, which is identical in behavior to a line
		///   strip in that at least 2 vertices must be added, (really 3 for a visible loop). Each vertex added after the
		///   first will create a line segment to the vertex added previously. A final line segment will be created
		///   automatically from the last added vertex to the very first vertex which will complete the loop.
		///
		void SetAsLineLoop(void) { mPrimitiveType = tbImplementation::Renderer::kLineLoop; }

	private:
		std::vector<tbCore::uint32> CreateIndices(void) const;

		std::vector<iceCore::MeshVertex> mVertices;
		tbImplementation::Renderer::PrimitiveType mPrimitiveType;
	};

};	//namespace Render

#endif /* ludumdare56_headless_build */
#endif /* Undefined_MeshCreator_hpp */
