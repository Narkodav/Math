#pragma once
#include "Math/Common.h"
#include "Math/Tensors/Tensors.h"

#include <random>
#include <array>
#include <cmath>

namespace Math
{
	class PerlinNoise3d
	{
	private:
		static constexpr int64_t s_gridCellSize = 64;
		static constexpr int64_t s_gradientTableSize = 256;
		static constexpr uint64_t s_permutationTableSize = 256;

		std::array<FVec3, s_gradientTableSize> m_gradientTable;
		std::array<uint64_t, s_permutationTableSize * 2> m_permutationTable;
		uint64_t m_seed = 0;
		std::mt19937 m_rng;

		void generateGradientTable() //allocates on equal int64_tervals on a sphere
		{
			int64_t root = std::sqrt(s_gradientTableSize);
			float azimuthalFraction = 2 * s_pi<float> / root;
			float polarFraction = 2 * s_pi<float> / root;
			float azimuthalAngle = 0.0f;
			float polarAngle = 0.0f;
			for (int64_t i = 0; i < root; i++)
			{
				azimuthalAngle = 0.0f;
				for (int64_t j = 0; j < root; j++)
				{
					m_gradientTable[i * root + j] = FVec3(std::cos(azimuthalAngle) * std::sin(polarAngle),
						std::sin(azimuthalAngle) * std::sin(polarAngle),
						std::cos(polarAngle));
					azimuthalAngle += azimuthalFraction;
				}
				polarAngle += azimuthalFraction;
			}
			std::shuffle(m_gradientTable.begin(), m_gradientTable.end(), m_rng);
		}

		void generatePermutationTableTable() //allocates on equal int64_tervals on a sphere
		{
			for (int64_t i = 0; i < s_permutationTableSize; i++)
				m_permutationTable[i] = i;

			std::shuffle(m_permutationTable.begin(), m_permutationTable.begin() + s_permutationTableSize, m_rng);

			for (uint64_t i = 0; i < s_permutationTableSize; i++)
				m_permutationTable[s_permutationTableSize + i] = m_permutationTable[i];
		}

		inline int64_t hash(int64_t x, int64_t y, int64_t z) const {
			return m_permutationTable[m_permutationTable[m_permutationTable[x & 255] + (y & 255)] + (z & 255)];
		}

	public:

		void setSeed(uint64_t newSeed)
		{
			m_seed = newSeed;
			m_rng.seed(m_seed);
			generateGradientTable();
			generatePermutationTableTable();
		}

		inline FVec3 getGradient(int64_t x, int64_t y, int64_t z) const
		{
			return m_gradientTable[hash(x, y, z)];
		}

		inline float getDot(FVec3 v, int64_t X, int64_t Y, int64_t Z) const
		{
			return Math::dot(getGradient(X, Y, Z), v);
		}

		float getOctave(float x, float y, float z) const
		{
			int64_t xCellCoord = x;
			int64_t yCellCoord = y;
			int64_t zCellCoord = z;
			if (x < 0) xCellCoord -= 1;
			if (y < 0) yCellCoord -= 1;
			if (z < 0) zCellCoord -= 1;

			float X = x - xCellCoord;
			float Y = y - yCellCoord;
			float Z = z - zCellCoord;

			float dotTopRightFront = getDot(FVec3(1 - X, 1 - Y, 1 - Z), xCellCoord + 1, yCellCoord + 1, zCellCoord + 1);
			float dotTopRightBack = getDot(FVec3(1 - X, 1 - Y, Z), xCellCoord + 1, yCellCoord + 1, zCellCoord);

			float dotTopLeftFront = getDot(FVec3(X, 1 - Y, 1 - Z), xCellCoord, yCellCoord + 1, zCellCoord + 1);
			float dotTopLeftBack = getDot(FVec3(X, 1 - Y, Z), xCellCoord, yCellCoord + 1, zCellCoord);

			float dotBottomRightFront = getDot(FVec3(1 - X, Y, 1 - Z), xCellCoord + 1, yCellCoord, zCellCoord + 1);
			float dotBottomRightBack = getDot(FVec3(1 - X, Y, Z), xCellCoord + 1, yCellCoord, zCellCoord);

			float dotBottomLeftFront = getDot(FVec3(X, Y, 1 - Z), xCellCoord, yCellCoord, zCellCoord + 1);
			float dotBottomLeftBack = getDot(FVec3(X, Y, Z), xCellCoord, yCellCoord, zCellCoord);

			float u = fade(X);
			float v = fade(Y);
			float w = fade(Z);

			float lerpFront = lerp(lerp(dotBottomLeftFront, dotBottomRightFront, u), lerp(dotTopLeftFront, dotTopRightFront, u), v);
			float lerpBack = lerp(lerp(dotBottomLeftBack, dotBottomRightBack, u), lerp(dotTopLeftBack, dotTopRightBack, u), v);

			return lerp(lerpBack, lerpFront, w);
		}

		float getFbm(float x, float y, float z, int64_t octaves, float frequency = 1.0f, float amplitude = 1.0f, float persistence = 0.5f, float lacunarity = 2.0f) const
		{
			float total = 0.0f;
			float maxValue = 0.0f;  // Used for normalizing the result

			for (int64_t i = 0; i < octaves; i++)
			{
				// Get noise value at current frequency
				total += getOctave(x * frequency, y * frequency, z * frequency) * amplitude;

				maxValue += amplitude;
				amplitude *= persistence;  // Amplitude decreases with each octave
				frequency *= lacunarity;  // Frequency increases with each octave
			}
			// if (abs(total) > 1)
			// 	__debugbreak();
			// Normalize the result to keep it roughly in [-1, 1] range
			return total / maxValue;
		}

	};
}