///
/// @file
/// @details Provides a very simple container of objects in an array that keeps overwriting itself as you go.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef TyreBytes_RingBuffer_hpp
#define TyreBytes_RingBuffer_hpp

#include <array>

namespace TyreBytes
{
	namespace Core
	{
		//NOTE: This was designed/developed for the PingMonitor, and might not fit the generic case of a RingBuffer
		//  as closely as desired for a generic container.
		//
		// buffer[0] will always be the most recent push and buffer[size - 1] will be the oldest.
		template <typename Type, size_t Size> class RingBuffer
		{
			//static_assert(std::is_pod<Type>::value, "Complex types are not supported.");
			static_assert(std::is_standard_layout<Type>::value && std::is_trivial<Type>::value, "Complex types are not supported.");


		public:
			RingBuffer(void) :
				mBuffer(),
				mWritePosition(0),
				mNumberOfItems(0)
			{
			}

			void Clear(void)
			{
				mWritePosition = 0;
				mNumberOfItems = 0;
			}

			void Push(const Type& thing)
			{
				mBuffer[mWritePosition] = thing;
				++mWritePosition;
				if (mWritePosition >= mBuffer.size())
				{
					mWritePosition = 0;
				}

				if (mNumberOfItems < mBuffer.size())
				{
					++mNumberOfItems;
				}
			}

			size_t size(void) const { return mNumberOfItems; }

			const Type& operator[](size_t index) const
			{
				return mBuffer[(mBuffer.size() + mWritePosition + index) % mBuffer.size()];
			}

			Type& operator[](size_t index)
			{
				return mBuffer[(mBuffer.size() + mWritePosition + index) % mBuffer.size()];
			}

		private:
			std::array<Type, Size> mBuffer;
			size_t mWritePosition;
			size_t mNumberOfItems;
		};

	};	//namespace Core
};	//namespace TyreBytes

#endif /* TyreBytes_RingBuffer_hpp */
