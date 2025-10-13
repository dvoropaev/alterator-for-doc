#!/usr/bin/python3
#
# Copyright (C) 2025 Pavel Khromov <hromovpi@altlinux.org>
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
#

import batch_info

import subprocess
import tomlkit
import re

SYSTEMINFO_BIN = "/usr/lib/alterator/backends/systeminfo"


def systeminfo(command: str) -> str:
    return (
        subprocess.run([SYSTEMINFO_BIN, command], capture_output=True)
        .stdout.decode()
        .strip()
    )


def get_arch_from_system() -> str:
    return systeminfo("arch")


def get_language_from_system() -> str:
    return systeminfo("locale").split("_")[0]


def get_desktops_from_system() -> str:
    return systeminfo("list-desktop-environments").split("\n")[0]


def get_kflavour_from_system() -> str:
    kernel = systeminfo("kernel")
    match = re.search(r"[^-]*-(.*)-[^-]*", kernel)
    return match[1]


def transform_package_name(name, properties, kflavour):
    result = name
    if "kernel_module" in properties:
        result = name + "-" + kflavour
    return result


def main():
    arch = get_arch_from_system()
    language = get_language_from_system()
    desktops = get_desktops_from_system()
    kflavour = get_kflavour_from_system()

    batch_info_text = batch_info.get_info()
    batch_info_text = list(filter(None, batch_info_text.split("\0")))

    packages = list()
    filtered_packages = list()
    for i in range(0, len(batch_info_text)):
        data = tomlkit.parse(batch_info_text[i])
        package_data = data["packages"]

        for pkg_name, pkg_data in package_data.items():
            if not isinstance(package_data, dict):
                filtered_packages.append(
                    transform_package_name(pkg_name, pkg_data, kflavour)
                )
            pkg_archs = pkg_data.get("arch")

            if pkg_archs is not None:
                if arch not in pkg_archs:
                    filtered_packages.append(
                        transform_package_name(pkg_name, pkg_data, kflavour)
                    )

            pkg_lang = pkg_data.get("language")
            if pkg_lang is not None and pkg_lang != language:
                filtered_packages.append(
                    transform_package_name(pkg_name, pkg_data, kflavour)
                )

            pkg_desktops = pkg_data.get("desktops")
            if pkg_desktops is not None:
                if not any(d in desktops for d in pkg_desktops):
                    filtered_packages.append(
                        transform_package_name(pkg_name, pkg_data, kflavour)
                    )

            packages.append(transform_package_name(pkg_name, pkg_data, kflavour))

    rpm_run = subprocess.run(
        ["rpm", "-q", *packages],
        capture_output=True,
    )

    # Get 2'nd column
    rpm_result = rpm_run.stderr.decode()
    rpm_result = rpm_result.split("\n")
    rpm_result = list(map(str.split, rpm_result))
    rpm_result = [row for row in rpm_result if row]
    rpm_result = list(zip(*rpm_result))
    rpm_result = rpm_result + filtered_packages
    print(str.join("\n", list(rpm_result[1])))


if __name__ == "__main__":
    main()
