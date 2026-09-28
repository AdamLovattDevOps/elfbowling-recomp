# WebAssembly build (port/web/README.md). Needs Emscripten on PATH (emsdk_env).
#   make -C port web                 -> build/web/dist (EXE= the original exe, as for `make native`)
# The flags live in web/build.py so that the same build runs on hosts without make (Windows emsdk).
WEB_EXE ?= $(NATIVE_EXE)
WEB_ARGS ?=
.PHONY: web web-clean
web:
	python3 web/build.py --exe "$(WEB_EXE)" $(WEB_ARGS)
web-clean:
	rm -rf ../build/web
