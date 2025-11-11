#!/bin/bash

su - builder -c "cd $PWD && gear-rpm -ba"

mkdir -p /artefacts/RPMS
mkdir -p /artefacts/SRPMS
mv /home/builder/RPM/RPMS/noarch/*.rpm /artefacts/RPMS/
mv /home/builder/RPM/SRPMS/*.rpm /artefacts/SRPMS/

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PREV_DIR=$(pwd)

cd "$SCRIPT_DIR/../packages" || exit 1

mkdir -p build/.gear

cp ./rules build/.gear/
cp ./*.spec build/.gear/

cd build || exit 1
git init || exit 1
git config --local user.name 'Builder' || exit 1
git config --local user.email 'builder@localhost' || exit 1
git add ./.gear/* || exit 1
git commit -m 'Initial commit' || exit 1

chown -R builder:builder .

su - builder -c "cd $PWD && gear-rpm -ba" || exit 1

cd "$PREV_DIR" || exit 1

mkdir -p /var/www/html/noarch/{RPMS.classic,base}
cp /home/builder/RPM/RPMS/noarch/*.rpm /var/www/html/noarch/RPMS.classic/
cd /var/www/html/ || exit 1
genbasedir --create --progress --topdir=. noarch classic

echo "rpm http://127.0.0.1/ noarch classic" > /etc/apt/sources.list.d/test.list
mv /etc/apt/sources.list.d/alt.list /tmp/alt.list

cd "$PREV_DIR" || exit 1
