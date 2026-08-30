#!/usr/bin/env python3
"""Aggregate WER runner for VoiceTyperBench over a prepared corpus manifest.

Runs the bench executable once per manifest row, sums the reported word error
counts into a corpus-level WER and prints per-run timing. Use it to A/B decode
settings (e.g. --beam 1 vs --beam 5) on identical data.

Example:
  python tools/bench_corpus.py --bench build/Bench_cpu/VoiceTyperBench.exe \
      --manifest .bench_data/test-clean/manifest.tsv --beam 5 --mode record --vad on
"""

import argparse
import json
import pathlib
import statistics
import subprocess
import sys


def parse_args():
	parser = argparse.ArgumentParser(description=__doc__)
	parser.add_argument("--bench", required=True, help="path to VoiceTyperBench executable")
	parser.add_argument("--manifest", required=True, help="manifest.tsv from tools/prep_librispeech.py")
	parser.add_argument("--mode", choices=["record", "streaming"], default="record")
	parser.add_argument("--vad", choices=["on", "off"], default="on")
	parser.add_argument("--beam", type=int, default=1)
	parser.add_argument("--threads", type=int, default=0, help="0 = bench default (hardware threads)")
	parser.add_argument("--model", default=None, help="default: <bench dir>/stt_models/ggml-base.en.bin")
	parser.add_argument("--vad-model", default=None,
		help="default: <bench dir>/vad_models/ggml-silero-v5.1.2.bin")
	parser.add_argument("--limit", type=int, default=0, help="only run the first N rows (0 = all)")
	parser.add_argument("--jsonl", default=None, help="optional path to write per-file results")
	return parser.parse_args()


def main():
	args = parse_args()
	bench = pathlib.Path(args.bench).resolve()
	if not bench.is_file():
		sys.exit(f"bench executable not found: {bench}")

	manifest_dir = pathlib.Path(args.manifest).resolve().parent
	rows = []
	for line in pathlib.Path(args.manifest).read_text(encoding="utf-8").splitlines():
		line = line.strip()
		if not line or line.startswith("#"):
			continue
		rel, _, text = line.partition("\t")
		if rel and text:
			rows.append((rel, text))
	if not rows:
		sys.exit("manifest is empty")
	if args.limit > 0:
		rows = rows[: args.limit]

	model = args.model or str(bench.parent / "stt_models" / "ggml-base.en.bin")
	vad_model = args.vad_model or str(bench.parent / "vad_models" / "ggml-silero-v5.1.2.bin")

	jsonl_file = open(args.jsonl, "w", encoding="utf-8") if args.jsonl else None

	total_sub = total_del = total_ins = total_ref = 0
	times = []
	failed = 0
	for i, (rel, text) in enumerate(rows):
		audio = (manifest_dir / rel).resolve()
		if not audio.is_file():
			print(f"skip (missing audio): {rel}", file=sys.stderr)
			continue

		cmd = [str(bench),
			"--audio", str(audio),
			"--expected-text", text,
			"--mode", args.mode,
			"--vad", args.vad,
			"--beam", str(args.beam),
			"--warmup", "0",
			"--iterations", "1",
			"--model", model,
			"--vad-model", vad_model,
			"--log", "off"]
		if args.threads > 0:
			cmd += ["--threads", str(args.threads)]

		proc = subprocess.run(cmd, capture_output=True, text=True)
		if proc.returncode != 0:
			print(f"bench failed ({proc.returncode}) on {rel}: {proc.stderr.strip()}", file=sys.stderr)
			failed += 1
			continue

		try:
			result = json.loads(proc.stdout)
		except json.JSONDecodeError:
			print(f"skip (bad json) on {rel}", file=sys.stderr)
			failed += 1
			continue

		total_sub += result["wer_substitutions"]
		total_del += result["wer_deletions"]
		total_ins += result["wer_insertions"]
		total_ref += result["wer_ref_words"]
		if result.get("transcribe_ms"):
			times.append(sum(result["transcribe_ms"]))

		if jsonl_file:
			result["file"] = rel
			jsonl_file.write(json.dumps(result) + "\n")

		done = i + 1
		if done % 25 == 0 or done == len(rows):
			err = total_sub + total_del + total_ins
			print(f"  {done}/{len(rows)}  WER so far: {err}/{total_ref} = {err / max(1, total_ref):.4f}",
				file=sys.stderr, flush=True)

	if jsonl_file:
		jsonl_file.close()

	errors = total_sub + total_del + total_ins
	print(json.dumps({
		"manifest": str(args.manifest),
		"files": len(rows) - failed,
		"failed": failed,
		"mode": args.mode,
		"vad": args.vad,
		"beam": args.beam,
		"threads": args.threads if args.threads > 0 else "default",
		"wer": errors / total_ref if total_ref > 0 else 1.0,
		"ref_words": total_ref,
		"substitutions": total_sub,
		"deletions": total_del,
		"insertions": total_ins,
		"transcribe_ms_mean": statistics.mean(times) if times else None,
		"transcribe_ms_median": statistics.median(times) if times else None,
	}))


if __name__ == "__main__":
	main()
