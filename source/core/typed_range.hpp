///
/// @file
/// @details Provides a ranged-based foreach access to arrays like RacecarState.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef TyreBytes_TypedRange_hpp
#define TyreBytes_TypedRange_hpp

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/core/tb_dynamic_structure.hpp>

#include <fstream>
#include <vector>

namespace TyreBytes
{
	namespace Core
	{

		template<typename IndexType, typename ValueType, size_t kMaximumIndex, ValueType& (*AccessFunction)(const IndexType index)> struct TypedRange
		{
			class Iterator
			{
			public:
				explicit Iterator(const IndexType index) :
					mIndex(index)
				{
				}

				Iterator operator++(void) { ++mIndex; return *this; }
				Iterator operator++(int) { Iterator value = *this; ++mIndex; return value; }
				bool operator==(const Iterator& other) const { return mIndex == other.mIndex; }
				bool operator!=(const Iterator& other) const { return mIndex != other.mIndex; }
				ValueType& operator*(void) const { return AccessFunction(mIndex); }

			private:
				IndexType mIndex;
			};

			Iterator begin(void) const { return Iterator{ 0 }; }
			Iterator end(void) const { return Iterator{ kMaximumIndex }; }
		};


		template<typename IndexType, typename ValueType, IndexType (*SizeFunction)(void), ValueType& (*AccessFunction)(const IndexType index)> struct DynamicTypedRange
		{
			class Iterator
			{
			public:
				explicit Iterator(const IndexType index) :
					mIndex(index)
				{
				}

				Iterator operator++(void) { ++mIndex; return *this; }
				Iterator operator++(int) { Iterator value = *this; ++mIndex; return value; }
				bool operator==(const Iterator& other) const { return mIndex == other.mIndex; }
				bool operator!=(const Iterator& other) const { return mIndex != other.mIndex; }
				ValueType& operator*(void) const { return AccessFunction(mIndex); }

			private:
				IndexType mIndex;
			};

			Iterator begin(void) const { return Iterator{ 0 }; }
			Iterator end(void) const { return Iterator{ SizeFunction() }; }
		};

	};	//namespace Core
};	//namespace TyreBytes

#endif /* TyreBytes_TypedRange_hpp */
