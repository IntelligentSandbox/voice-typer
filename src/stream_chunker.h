#pragma once

#include "build_time_constants.h"

#include <cmath>
#include <vector>

// Shared speech/silence classifier for the streaming segmenter. The live segmenter
// thread (audio_pipeline.h) and the offline replica below feed poll-window RMS
// values through the same detector so both produce identical chunk boundaries.
struct StreamSpeechDetector
{
	float NoiseFloor = STREAM_SPEECH_RMS_THRESHOLD / STREAM_SPEECH_RMS_RATIO;
	int SilenceMs = 0;
	bool HasSpeech = false;
};

static float
stream_speech_threshold(const StreamSpeechDetector &Detector)
{
	float Threshold = Detector.NoiseFloor * STREAM_SPEECH_RMS_RATIO;
	if (Threshold < STREAM_SPEECH_RMS_MIN) Threshold = STREAM_SPEECH_RMS_MIN;
	if (Threshold > STREAM_SPEECH_RMS_MAX) Threshold = STREAM_SPEECH_RMS_MAX;
	return Threshold;
}

// Feed one poll window's RMS; advances SilenceMs and the adaptive noise floor.
// Returns true when the window is classified as speech.
static bool
stream_speech_detector_poll(StreamSpeechDetector *Detector, float CurrentRms, int PollIntervalMs)
{
	if (CurrentRms >= stream_speech_threshold(*Detector))
	{
		Detector->HasSpeech = true;
		Detector->SilenceMs = 0;
		return true;
	}

	if (Detector->HasSpeech)
	{
		Detector->SilenceMs += PollIntervalMs;
	}
	else
	{
		float Alpha = CurrentRms > Detector->NoiseFloor ? STREAM_NOISE_FLOOR_RISE : STREAM_NOISE_FLOOR_DECAY;
		Detector->NoiseFloor += (CurrentRms - Detector->NoiseFloor) * Alpha;
	}

	return false;
}

// Offline replica of stream_segment_thread's chunking logic. Operates on a complete
// PCM buffer and returns the silence-bounded chunks the live segmenter would have
// produced, including the final flush on stop (equivalent to StreamingFinalizeOnStop).
static std::vector<std::vector<float>>
chunk_audio_for_streaming(const std::vector<float> &Samples, int SampleRate)
{
	auto compute_rms_block = [](const float *S, int N) -> float {
		if (N <= 0) return 0.0f;
		double Sum = 0.0;
		for (int i = 0; i < N; i++)
		{
			Sum += (double)S[i] * (double)S[i];
		}
		return (float)std::sqrt(Sum / (double)N);
	};

	std::vector<std::vector<float>> Chunks;
	int Total = (int)Samples.size();
	if (Total <= 0) return Chunks;

	int PollSamples = SampleRate * STREAM_POLL_INTERVAL_MS / 1000;
	if (PollSamples <= 0) PollSamples = 1;

	StreamSpeechDetector Detector;
	std::vector<float> Accum;
	Accum.reserve(Total);

	int Pos = 0;
	while (Pos < Total)
	{
		int Remaining = Total - Pos;
		int RecentCount = PollSamples < Remaining ? PollSamples : Remaining;
		const float *BlockStart = Samples.data() + Pos;
		float CurrentRms = compute_rms_block(BlockStart, RecentCount);
		Pos += RecentCount;

		Accum.insert(Accum.end(), BlockStart, BlockStart + RecentCount);

		bool IsSpeech = stream_speech_detector_poll(&Detector, CurrentRms, STREAM_POLL_INTERVAL_MS);
		if (!IsSpeech && !Detector.HasSpeech)
		{
			Accum.clear();
			continue;
		}

		int BufferDurationMs = (int)Accum.size() * 1000 / SampleRate;
		if (Detector.HasSpeech &&
			Detector.SilenceMs >= STREAM_SILENCE_DURATION_MS &&
			BufferDurationMs >= STREAM_MIN_CHUNK_DURATION_MS)
		{
			Chunks.push_back(std::move(Accum));
			Accum.clear();
			Accum.reserve(Total - Pos);
			Detector.SilenceMs = 0;
			Detector.HasSpeech = false;
		}
	}

	int BufferDurationMs = (int)Accum.size() * 1000 / SampleRate;
	if (Detector.HasSpeech && BufferDurationMs >= STREAM_MIN_CHUNK_DURATION_MS) Chunks.push_back(std::move(Accum));

	return Chunks;
}
