#!/usr/bin/env python3
"""Prepare a LibriSpeech subset for WER benchmarking with VoiceTyperBench.

Takes an extracted LibriSpeech split directory (e.g. LibriSpeech/test-clean),
converts an evenly spaced sample of utterances to 16 kHz mono PCM16 WAV and
writes a manifest.tsv (relative wav path TAB reference transcript) usable by
tools/bench_corpus.py.

Example:
  python tools/prep_librispeech.py --src ~/tmp/LibriSpeech/test-clean \
      --out .bench_data/test-clean --count 150
"""

import argparse
import pathlib
import subprocess
import sys


def parse_args():
	parser = argparse.ArgumentParser(description=__doc__)
	parser.add_argument("--src", required=True, help="extracted LibriSpeech split directory")
	parser.add_argument("--out", required=True, help="output directory (manifest.tsv + wav/)")
	parser.add_argument("--count", type=int, default=150, help="number of utterances to sample")
	parser.add_argument("--min-duration-sec", type=float, default=2.0,
		help="skip converted utterances shorter than this (streaming-mode chunks need >= 1s)")
	parser.add_argument("--seed-note", default="", help="free-form note appended to the manifest header")
	return parser.parse_args()


def main():
	args = parse_args()
	src = pathlib.Path(args.src)
	out = pathlib.Path(args.out)
	if not src.is_dir():
		sys.exit(f"source directory not found: {src}")

	entries = []
	for trans in sorted(src.rglob("*.trans.txt")):
		for line in trans.read_text(encoding="utf-8").splitlines():
			line = line.strip()
			if not line:
				continue
			utt_id, _, text = line.partition(" ")
			flac = trans.parent / (utt_id + ".flac")
			if flac.is_file():
				entries.append((utt_id, flac, text))

	if not entries:
		sys.exit(f"no utterances found under {src}")

	step = max(1, len(entries) // max(1, args.count))
	picked = entries[::step][: args.count]

	wav_dir = out / "wav"
	wav_dir.mkdir(parents=True, exist_ok=True)

	rows = []
	converted = 0
	for utt_id, flac, text in picked:
		wav = wav_dir / (utt_id + ".wav")
		cmd = ["ffmpeg", "-loglevel", "error", "-y", "-i", str(flac),
			"-ar", "16000", "-ac", "1", "-c:a", "pcm_s16le", str(wav)]
		if subprocess.run(cmd).returncode != 0:
			print(f"skip (ffmpeg failed): {utt_id}", file=sys.stderr)
			continue

		probe = ["ffprobe", "-v", "error", "-show_entries", "format=duration",
			"-of", "default=noprint_wrappers=1:nokey=1", str(wav)]
		probe_out = subprocess.run(probe, capture_output=True, text=True).stdout.strip()
		try:
			duration = float(probe_out)
		except ValueError:
			print(f"skip (bad duration): {utt_id}", file=sys.stderr)
			continue
		if duration < args.min_duration_sec:
			wav.unlink()
			continue

		rows.append((f"wav/{wav.name}", text))
		converted += 1

	total_seconds = 0.0
	for rel, _ in rows:
		wav = out / rel
		probe = ["ffprobe", "-v", "error", "-show_entries", "format=duration",
			"-of", "default=noprint_wrappers=1:nokey=1", str(wav)]
		total_seconds += float(subprocess.run(probe, capture_output=True, text=True).stdout.strip())

	with open(out / "manifest.tsv", "w", encoding="utf-8", newline="\n") as f:
		f.write(f"# source: {src}\n")
		f.write(f"# utterances: {converted}  total: {total_seconds / 60.0:.1f} min\n")
		if args.seed_note:
			f.write(f"# note: {args.seed_note}\n")
		for rel, text in rows:
			f.write(f"{rel}\t{text}\n")

	print(f"wrote {converted} utterances ({total_seconds / 60.0:.1f} min) to {out / 'manifest.tsv'}")


if __name__ == "__main__":
	main()
