#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
template="${root_dir}/subprojects/dbusxml-to-md-xslt-template/template.xsl"

subprojects=(
  "alterator-backend-packages"
  "alterator-backend-edition"
  "alterator-backend-component"
)

for subproject in "${subprojects[@]}"; do
  subproject_dir="${root_dir}/subprojects/${subproject}"
  docs_dir="${subproject_dir}/docs"
  mkdir -p "${docs_dir}"

  find "${subproject_dir}" -type f -name 'org.altlinux.alterator.*.xml' | while read -r xml_file; do
    file_name="$(basename "${xml_file}")"
    interface_name="${file_name%.xml}"
    short_name="${interface_name#org.altlinux.alterator.}"

    tmp_en="$(mktemp)"
    tmp_ru="$(mktemp)"

    xsltproc --novalid "${template}" "${xml_file}" > "${tmp_en}"
    xsltproc --novalid --stringparam lang ru_RU "${template}" "${xml_file}" > "${tmp_ru}"

    {
      printf '[English](./%s.md) | [Русский](./%s.ru_RU.md)\n\n' "${short_name}" "${short_name}"
      cat "${tmp_en}"
      printf '\n\nCurrent specification: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc\n'
    } > "${docs_dir}/${short_name}.md"

    {
      printf '[English](./%s.md) | [Русский](./%s.ru_RU.md)\n\n' "${short_name}" "${short_name}"
      cat "${tmp_ru}"
      printf '\n\nАктуальная спецификация: https://altlinux.space/alterator/alterator-entry/src/branch/master/doc\n'
    } > "${docs_dir}/${short_name}.ru_RU.md"

    rm -f "${tmp_en}" "${tmp_ru}"
  done
done
