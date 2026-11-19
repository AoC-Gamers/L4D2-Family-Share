.DEFAULT_GOAL := help

ifeq ($(OS),Windows_NT)
PYTHON ?= python
SMX_PLATFORM ?= windows
SPCOMP_NAME := spcomp.exe
else
PYTHON ?= $(shell command -v python3 >/dev/null 2>&1 && echo python3 || echo python)
SMX_PLATFORM ?= linux
SPCOMP_NAME := spcomp
endif

# Support make -f from a different directory, including paths that contain spaces.
ROOT_DIR := $(shell $(PYTHON) -c "import sys; from pathlib import Path; print(Path(sys.argv[1]).resolve().parent.as_posix())" "$(MAKEFILE_LIST)")

SOURCEMOD_VERSION ?= 1.12
SPCOMP ?= .deps/smx/sourcemod-$(SMX_PLATFORM)/addons/sourcemod/scripting/$(SPCOMP_NAME)
SMX_BUILD_DIR ?= .build/smx
SMX_PACKAGE_DIR ?= .build/package-smx
SMX_COMPILE_LOG ?= .build/build-smx-compile.log
SOURCEMOD_ARTIFACT_DIR ?= dist/sourcemod/artifact
RELEASE_BASENAME ?= l4d2-family-share-local
RUN = cd "$(ROOT_DIR)" && $(PYTHON)

.PHONY: help stage-smx deps-smx build-smx package-smx validate-artifact release clean clean-all
help:
	@$(RUN) -c "print('deps-smx, build-smx, package-smx, release, validate-artifact, clean, clean-all, test-ci, test-offline')"

deps-smx:
	$(RUN) scripts/deps/fetch-sourcemod.py --root . --platform "$(SMX_PLATFORM)" --version "$(SOURCEMOD_VERSION)" --archive-url "$(SOURCEMOD_ARCHIVE_URL)"

build-smx:
	$(RUN) scripts/build/build-local.py --root . --spcomp "$(SPCOMP)" --output-root "$(SMX_BUILD_DIR)" --compile-log "$(SMX_COMPILE_LOG)"

package-smx:
	$(RUN) scripts/package/stage-artifact.py . "$(SMX_BUILD_DIR)" "$(SMX_COMPILE_LOG)" "$(SMX_PACKAGE_DIR)"

stage-smx:
	$(RUN) scripts/package/stage-artifact.py . "$(SMX_PACKAGE_DIR)" "$(SMX_COMPILE_LOG)" "$(SOURCEMOD_ARTIFACT_DIR)"

release: stage-smx
	$(RUN) scripts/package/package-release.py --root . --artifact "$(SOURCEMOD_ARTIFACT_DIR)" --basename "$(RELEASE_BASENAME)"

validate-artifact:
	$(RUN) scripts/package/artifact_contract.py --root . --artifact "$(SOURCEMOD_ARTIFACT_DIR)"

clean:
	$(RUN) -c "import shutil; shutil.rmtree('.build', ignore_errors=True); shutil.rmtree('dist', ignore_errors=True)"

clean-all: clean
	$(RUN) -c "import shutil; shutil.rmtree('.deps', ignore_errors=True)"

.PHONY: test-offline
test-offline:
	$(RUN) -m unittest discover -s tests -v

.PHONY: test-ci
test-ci:
	$(RUN) -m unittest discover -s tests -p test_artifact_manifest.py -v

RELEASE_TAG ?= $(GITHUB_REF_NAME)
.PHONY: resolve-release
resolve-release:
	@cd "$(ROOT_DIR)" && bash scripts/ci/ci-resolve-release.sh "$(RELEASE_TAG)"

RELEASE_VERSION ?=
RELEASE_NOTES_PATH ?= dist/release/release-notes.md
.PHONY: release-notes
release-notes:
	cd "$(ROOT_DIR)" && bash scripts/ci/ci-extract-changelog-release-notes.sh "$(RELEASE_VERSION)" "$(RELEASE_NOTES_PATH)"
