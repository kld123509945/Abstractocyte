#ifndef GRID3D_H_
#define GRID3D_H_


#include <stdint.h>
#include <cassert>
#include <cmath>
#include <cstring> 
#include <limits>
#include <vector>

struct VertexData;

namespace SpacePartitioning
{
	class Grid3D
	{
	public:
		Grid3D();
		Grid3D(int sizeX, int sizeY, int sizeZ);
		~Grid3D();

		void addNormalizedPoint(float x, float y, float z, float value);
		void setSize(int sizeX, int sizeY, int sizeZ);
		void reset();
		float* getData();
		unsigned char* getData8Bit();
	protected:

		float m_maxValue;
		int m_size[3];

		float* m_data;
		unsigned char* m_data_8bit;

	};
}

#endif /* GRID3D_H_ */