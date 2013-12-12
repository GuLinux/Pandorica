deps_qt4="QTCORE4.DLL
QTGUI4.DLL
QTNETWORK4.DLL"

deps_qt5="QT5CORE.DLL
QT5GUI.DLL
QT5NETWORK.DLL
QT5WIDGETS.DLL
"

deps_qt="$deps_qt4"
if test "$USE_QT5" == "true"; then
  deps_qt="$deps_qt5"
fi

deps="$deps_qt4
LIBICONV-2.DLL
AVCODEC-55.DLL
AVFORMAT-55.DLL
AVUTIL-52.DLL
LIBBOOST_CHRONO-MT.DLL
LIBBOOST_DATE_TIME-MT.DLL
LIBBOOST_FILESYSTEM-MT.DLL
LIBBOOST_PROGRAM_OPTIONS-MT.DLL
LIBBOOST_RANDOM-MT.DLL
LIBBOOST_REGEX-MT.DLL
LIBBOOST_SYSTEM-MT.DLL
LIBBOOST_THREAD_WIN32-MT.DLL
LIBGCC_S_SJLJ-1.DLL
LIBPNG16-16.DLL
LIBPQ.DLL
LIBSSP-0.DLL
LIBSTDC++-6.DLL
LIBWINPTHREAD-1.DLL
SWSCALE-2.DLL
LIBEAY32.DLL
LIBGLESV2.DLL
LIBINTL-8.DLL
LIBPCRE16-0.DLL
SSLEAY32.DLL
ZLIB1.DLL"

HOST_PREFIX="i686-w64-mingw32"
WORKDIR="/tmp/Pandorica-$HOST_PREFIX"
rm -rf "$WORKDIR"
mkdir "$WORKDIR"
for file in $deps; do find /usr/$HOST_PREFIX -iname $file -exec cp -av {} "$WORKDIR" \; ; done
cp -av files/html_templates/ "$WORKDIR"
cp -av files/static/ "$WORKDIR"
cp -av /usr/share/Wt/resources/ "$WORKDIR"
cp -av files/strings*xml "$WORKDIR"

cp $1 "$WORKDIR"

${HOST_PREFIX}-strip -x $WORKDIR/*.dll
${HOST_PREFIX}-strip -x $WORKDIR/*.exe

cd /tmp
zipname="/tmp/Pandorica-$HOST_PREFIX-$( date +%Y-%m-%d ).zip"
zip -9r "$zipname" Pandorica-$HOST_PREFIX
cd -
mv "$zipname" .
rm -rf "$WORKDIR"
