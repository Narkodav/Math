#pragma once
#include "Math/Common.h"
#include "Math/Tensors/Tensors.h"

#include <random>
#include <array>
#include <cmath>

namespace Math
{
	class PerlinNoise2d
	{
	private:
		static constexpr int64_t s_gradientTableSize = 256;
		std::array<FVec2, s_gradientTableSize> m_gradientTable;
		uint64_t m_seed = 0;
		std::mt19937 m_rng;

		void generateGradientTable() {
			float fraction = 2 * s_pi<float> / s_gradientTableSize;
			float angle = 0.0f;
			for (int64_t i = 0; i < s_gradientTableSize; i++) {
				m_gradientTable[i] = {std::cos(angle), std::sin(angle)};
				angle += fraction;
			}
			std::shuffle(m_gradientTable.begin(), m_gradientTable.end(), m_rng);
		}

		inline int64_t hash(int64_t x, int64_t y) const {
			int64_t h = x * 1619 + y * 31337 + m_seed;
			h = (h << 13) ^ h;
			return h * (h * h * 60493 + 19990303) + 1376312589;
		}

	public:

		void setSeed(uint64_t newSeed) {
			m_seed = newSeed;
			m_rng.seed(m_seed);
			generateGradientTable();
		}

		inline FVec2 getGradient(int64_t x, int64_t y) const {
			return m_gradientTable[hash(x, y) & (s_gradientTableSize - 1)];
		}

		inline float getDot(FVec2 v, int64_t X, int64_t Y) const {
			return dot(getGradient(X, Y), v);
		}

		float getOctave(float x, float y) const {
			int64_t xCellCoord = (int64_t)x;
			int64_t yCellCoord = (int64_t)y;
			if (x < 0)
				xCellCoord -= 1;
			if (y < 0)
				yCellCoord -= 1;

			float X = x - xCellCoord;
			float Y = y - yCellCoord;

			float dotTopRight = getDot(FVec2(1 - X, 1 - Y), xCellCoord + 1, yCellCoord + 1);
			float dotTopLeft = getDot(FVec2(X, 1 - Y), xCellCoord, yCellCoord + 1);
			float dotBottomRight = getDot(FVec2(1 - X, Y), xCellCoord + 1, yCellCoord);
			float dotBottomLeft = getDot(FVec2(X, Y), xCellCoord, yCellCoord);

			float u = fade(X);
			float v = fade(Y);

			return lerp(lerp(dotBottomLeft, dotBottomRight, u), lerp(dotTopLeft, dotTopRight, u), v);
		}

		float getFbm(float x, float y, int64_t octaves, float frequency = 1.0f, float amplitude = 1.0f, float persistence = 0.5f, float lacunarity = 2.0f) const {
			float total = 0.0f;
			float maxValue = 0.0f;  // Used for normalizing the result

			for (int64_t i = 0; i < octaves; i++)
			{
				// Get noise value at current frequency
				total += getOctave(x * frequency, y * frequency) * amplitude;

				maxValue += amplitude;
				amplitude *= persistence;  // Amplitude decreases with each octave
				frequency *= lacunarity;  // Frequency increases with each octave
			}

			// Normalize the result to keep it roughly in [-1, 1] range
			return total / maxValue;
		}
	};
}
