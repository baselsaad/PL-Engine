#pragma once

namespace PAL
{
	class DeltaTime
	{
	public:
		DeltaTime() = default;
		
		void Update();

		inline float GetDeltaInMilliSeconds() const { return m_DeltaTime.FrameTime * 1000.0f; }
		inline float GetDeltaInSeconds() const { return m_DeltaTime.FrameTime; }

		inline int GetAverageFPS() const { return m_AvgData.FPS; }
		inline float GetAvgDeltaTimeInSeconds() const { return m_AvgData.AvgFrameTime; }
		inline float GetAvgDeltaTimeInMilliSeconds() const { return m_AvgData.AvgFrameTime * 1000.0f; }
	private:
		
		struct DeltaTimeData
		{
			float FrameTime = 0.0f;		// DeltaTime
			float LastFrameTime = 0.0f;
		} m_DeltaTime;

		struct AvgData
		{
			float LastFrameTime = 0.0f;
			float AvgFrameTime = 0.0f;
			float TotalFrameTimeInOneSec = 0.0f;

			int FPS = 0;

			int FrameCount = 0;
		} m_AvgData;

	};

}