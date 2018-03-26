#!/bin/sh

if [ $# -ne 1 ]
then
	echo "Usage: $0 <release_number>"
	exit 1;
fi

JSON="package_sukkopera_mytrackr_index.json"
TEMPLATE="$JSON.TEMPLATE"
BRD_VER="1.0"
RELEASE="$1"
ZIP="mytrackr-$BRD_VER-brdpkg-$RELEASE.zip"

zip -r $ZIP MyTrackr/
hash=$(sha256sum $ZIP | cut -f 1 -d " ")
size=$(ls -l $ZIP | awk '{print $5}')

cat $TEMPLATE | \
	sed "/^#/ d" | \
	sed "s/%BOARDVERSION%/$BRD_VER/g" | \
	sed "s/%VERSION%/$RELEASE/g" | \
	sed "s/%HASH%/$hash/g" | \
	sed "s/%SIZE%/$size/g" \
	> $JSON
