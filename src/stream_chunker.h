#pragma once

#include <cmath>
#include <vector>

// Minimum RMS energy to bother sending a chunk to whisper.
#define PIPELINE_SILENCE_RMS_THRESHOLD 0.0005f

// How often (ms) the stream segmenter thread polls the audio buffer for energy levels.
#define STREAM_POLL_INTERVAL_MS 100

// Cold-start RMS energy threshold for classifying a poll interval as speech vs
// silence. Once capture starts the threshold adapts to the measured noise floor.
#define STREAM_SPEECH_RMS_THRESHOLD 0.002f

// The speech threshold tracks the measured noise floor: threshold = floor * RATIO,
// clamped to [MIN, MAX] so digital silence cannot drive it to zero and loud rooms
// cannot push it past reasonable speech levels.
#define STREAM_SPEECH_RMS_RATIO 2.5f
#define STREAM_SPEECH_RMS_MIN 0.0015f
#define STREAM_SPEECH_RMS_MAX 0.02f

// Noise floor tracking rates, applied per poll while no speech is in progress:
// rises quickly to follow noise ramps, decays slowly so brief quiet gaps do not
// collapse it under the next utterance.
#define STREAM_NOISE_FLOOR_RISE 0.3f
#define STREAM_NOISE_FLOOR_DECAY 0.05f

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

// Minimum chunk duration (ms) before a speech->silence transition can trigger a cutoff.
#define STREAM_MIN_CHUNK_DURATION_MS 1000

// How long silence (ms) must persist after speech before cutting the chunk.
#define STREAM_SILENCE_DURATION_MS 500

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
