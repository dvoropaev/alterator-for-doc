#!/bin/bash

su - builder -c "cd $PWD && gear-rpm -ba"

arch=$(uname -m)

rm -f /home/builder/RPM/RPMS/${arch}/*-debuginfo-*.rpm

mkdir -p /artefacts/RPMS
mkdir -p /artefacts/SRPMS

if [ -d "/home/builder/RPM/RPMS/${arch}" ]; then
    mv /home/builder/RPM/RPMS/${arch}/*.rpm /artefacts/RPMS/
else
    echo "Error: ${arch} directory not found"
    exit 1
fi

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

mkdir -p /var/www/html/${arch}/{RPMS.classic,base}
mkdir -p /var/www/html/noarch/{RPMS.classic,base}
cp /home/builder/RPM/RPMS/${arch}/*.rpm /var/www/html/${arch}/RPMS.classic/
cp /home/builder/RPM/RPMS/noarch/*.rpm /var/www/html/noarch/RPMS.classic/
cd /var/www/html/ || exit 1
genbasedir --create --progress --topdir=. ${arch} classic
genbasedir --create --progress --topdir=. noarch classic

echo "rpm http://127.0.0.1/ ${arch} classic" > /etc/apt/sources.list.d/test.list
echo "rpm http://127.0.0.1/ noarch classic" >> /etc/apt/sources.list.d/test.list
mv /etc/apt/sources.list.d/alt.list /tmp/alt.list

cd "$PREV_DIR" || exit 1
