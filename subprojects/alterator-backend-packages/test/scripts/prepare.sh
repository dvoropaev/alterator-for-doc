#!/bin/bash

remove_version() {
  echo "$@" | awk '{
      for (i=1; i<=NF; i++) {
          split($i, a, /[<>=]/)
          if (a[1] ~ /^%/) next
          if (a[1] == "") next
          print a[1]
      }
  }'
}

echo "Installing all required packages"

apt-get update
apt-get install -y su jq gear rpmspec rpm-build apt-repo-tools apache2-base

export SPEC=`grep -E '^\s*spec\s*:' .gear/rules | sed -E 's/^\s*spec\s*:\s*//; s/\s*#.*//; s/^\s+|\s+$//g'`;
export REQUIRES=`remove_version "$(rpmspec -q --requires "$SPEC")"`
export BUILDREQUIRES=`remove_version $(rpmspec -q --buildrequires "$SPEC")`
apt-get install -y $REQUIRES $BUILDREQUIRES

echo "Adding user builder"

adduser builder
usermod -aG rpm builder

mkdir -p /home/builder/RPM
chown -R builder:builder $PWD /home/builder