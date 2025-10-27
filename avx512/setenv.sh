# Set/unset the paths for the rebuilt OpenSSL.
_ROOT=$(cd $(dirname "$BASH_SOURCE[0]"); pwd)
_VERSION=
_REMOVE=false
for arg in "$@"; do
    case $arg in
        -u) _REMOVE=true;;
        -*) echo >&2 "invalid option: $arg"; return 0;;
        *)  _VERSION="$arg"
    esac
done
_ROOTVER="$_ROOT/openssl-$_VERSION"

if $_REMOVE; then
    for var in PATH LD_LIBRARY_PATH; do
        export $var=$(echo "${!var}" | tr : '\n' | grep -v -e "^$_ROOT" -e '$^' | tr '\n' : | sed -e 's/:*$//')
    done
    unset CFLAGS
    unset CXXFLAGS
    unset LDFLAGS
elif [[ -z $_VERSION ]]; then
    echo >&2 "no version defined"
elif [[ ! -d "$_ROOTVER" ]]; then
    echo >&2 "directory $_ROOTVER not found"
else
    _BIN="$_ROOTVER/bin"
    _LIB="$_ROOTVER/lib64"
    [[ -f "$_LIB/libcrypto.so" ]] || _LIB="$_ROOTVER/lib"
    export PATH="$_BIN":$(echo "$PATH" | tr : '\n' | grep -v -e "^$_ROOT" -e '$^' | tr '\n' : | sed -e 's/:*$//')
    export LD_LIBRARY_PATH=$(echo "$_LIB":$(echo "$LD_LIBRARY_PATH" | tr : '\n' | grep -v -e "^$_ROOT" -e '$^' | tr '\n' :) | sed -e 's/:*$//')
    export CFLAGS="-I$_ROOTVER/include"
    export CXXFLAGS="-I$_ROOTVER/include"
    export LDFLAGS="-L$_LIB"
fi
