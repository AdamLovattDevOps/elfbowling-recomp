PY := .venv/bin/python

.PHONY: map match xref verify progress assets native hires
map:
	$(PY) tools/libmatch.py
	$(PY) tools/funcs.py
match:
	$(PY) tools/match.py
xref:
	$(PY) tools/xref_check.py
# byte match plus fixup-target check: the full correctness gate
verify: match xref progress
progress:
	@$(PY) tools/progress.py
assets:
	$(PY) tools/unpack.py build/assets
# native macOS/Linux build (clang++, SDL2): build/elfbowl. -k: keep going for a full error list
native:
	$(MAKE) -k -C port native
# 3x AI-upscaled art for the native build: build/hires/x3 (docs/HIRES.md; needs realesrgan-ncnn-vulkan)
hires:
	python3 tools/upscale.py
