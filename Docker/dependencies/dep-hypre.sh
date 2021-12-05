#! /bin/bash
set -o errexit
set -o xtrace
set -o verbose

. ./common-functions.sh

# shellcheck disable=1091
. /opt/intel/oneapi/setvars.sh

HYPRE_SOURCE="https://github.com/hypre-space/hypre/archive/refs/tags/v2.23.0.zip"
HYPRE_ARCHIVE=$(basename $HYPRE_SOURCE)
HYPRE_PATH="hypre-2.23.0"

build_and_install() {
	local source=$1
	pushd "$source" || exit 1
	pushd src || exit 1
	./configure --prefix=/usr/local/
	make -j "$(nproc)"
	make install
	popd || exit 1
	popd || exit 1
}

main() {
	pushd "$BUILD_PATH" || exit 1
	download_source "$HYPRE_SOURCE"
	extract_source "$HYPRE_ARCHIVE"
	build_and_install "$HYPRE_PATH"
	popd || exit 1
}

main
