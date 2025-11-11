#!/bin/bash

/bin/sh -c 'exec /usr/sbin/httpd2 -DFOREGROUND -k start' &

apt-get update

if ! command -v jq >/dev/null 2>&1; then
  echo "Error: jq is not installed" >&2
  return 1
fi

validate_check_apply() {
  local args="$1"
  local expected="$2"

  if ! echo "$expected" | jq -e . >/dev/null 2>&1; then
      echo "\`apt-wrapper check-apply \"$args\"\`: FAILED - invalid JSON in expected" >&2
      echo "Expected: $expected" >&2
      echo "Got: $result" >&2
      return 1
  fi

  local result
  result=$( /usr/lib/alterator-backend-packages/apt-wrapper check-apply $args | \
    jq -c '.install_packages |= sort | .remove_packages |= sort | .extra_remove_packages |= sort' )

  if [ $? -ne 0 ]; then
    echo "\`apt-wrapper check-apply \"$args\"\`: FAILED (apt-wrapper or jq failed)"
    return 1
  fi

  if echo "$result" | jq --argjson expected "$expected" -e '. == $expected' >/dev/null; then
    echo "\`apt-wrapper check-apply \"$args\"\`: PASSED"
    return 0
  else
    echo "\`apt-wrapper check-apply \"$args\"\`: FAILED - JSON mismatch" >&2
    echo "Expected: $expected" >&2
    echo "Got: $result" >&2
    return 1
  fi
}

validate_check_apply "package1" '{"install_packages": ["package1", "package2", "package3", "package4", "package5"], "remove_packages": [], "extra_remove_packages": []}' \
|| exit 1

apt-get install "package1" -y || exit 1

validate_check_apply "package6 package1-" '{"install_packages": ["package6"], "remove_packages": ["package1", "package2", "package3", "package4", "package5"], "extra_remove_packages": []}'\
|| exit 1
