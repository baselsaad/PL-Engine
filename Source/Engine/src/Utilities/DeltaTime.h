#pragma once

namespace PAL
{
	class DeltaTime
	{
	public:
		DeltaTime() 
			: m_FrameTimeIndex(0)
			, m_TotalFrameTime(0.0f)
			, m_AverageFrameTime(0.0f)
		{
			// Initialize the array with zeros
			std::fill(std::begin(m_FrameTimeArray), std::end(m_FrameTimeArray), 0.0f);
		}
	public:
		void Update();
		void CalculateAverage();

		inline double GetMilliSeconds() const { return m_FrameTime * 1000.0; }
		inline double GetSeconds() const { return m_FrameTime; }

		inline int GetFramePerSeconds() const
		{
			return m_FrameTime > 0.0 ? static_cast<int>(1.0f / m_FrameTime) : 0;
		}

		// Average calculations
		inline double GetAverageFrameTimeSeconds() const { return m_AverageFrameTime; }
		inline double GetAverageFrameTimeMili() const { return m_AverageFrameTime * 1000.0; }
		inline int GetAverageFPS() const
		{
			return m_AverageFrameTime > 0.0 ? static_cast<int>(1.0f / m_AverageFrameTime) : 0;
		}

	private:
		double m_FrameTime;                      // Most recent frame time
		double m_LastFrameTime = 0.0f;           // Time at the last frame

		// Average calculations
		static const size_t m_MaxSamples = 100;  // Number of frames to average over
		double m_FrameTimeArray[m_MaxSamples];   // Static array for frame times
		double m_AverageFrameTime;               // Average frame time
		size_t m_FrameTimeIndex;                 // Current index in the array
		double m_TotalFrameTime;                 // Total of all frame times in the array
	};

}