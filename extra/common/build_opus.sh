#/usr/bin/env zsh

OPUS_VERSION="1.2.1"

if ! [ -f "$CACHE_DIR/usr/lib/pkgconfig/opus.pc" ]; then
  curl https://ftp.osuosl.org/pub/xiph/releases/opus/opus-${OPUS_VERSION}.tar.gz -o opus.tar.gz
  tar xzf opus.tar.gz
  cd opus-${OPUS_VERSION}
  ./configure "$TARGET_HOST" \
              --prefix="$CACHE_DIR/usr" \
              --disable-extra-programs \
              --disable-doc
  make -j`nproc`
  make install
  cd ..
  rm -rf opus**
fi
