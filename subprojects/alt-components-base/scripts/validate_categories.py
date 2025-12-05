#!/bin/python3

import alterator_entry as ae
import os
import sys


CATEGORIES_DIR = "categories"
COMPONENTS_DIR = "components"
EDITIONS_DIR = "editions"

exit_code = 0


def error(msg: str) -> None:
    print(f"ERROR: {msg}", file=sys.stderr)
    global exit_code
    exit_code = 1


def get_all_files_with_extension(dir: str, extension: str) -> list[str]:
    found_files = []
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith("." + extension):
                path = os.path.join(root, file)
                ae.validate(path)
                found_files.append(path)
    return found_files


component_files = get_all_files_with_extension(COMPONENTS_DIR, "component")
vendors_component_files = get_all_files_with_extension(COMPONENTS_DIR, "vendors")
categories_files = get_all_files_with_extension(CATEGORIES_DIR, "category")
edition_files = get_all_files_with_extension(EDITIONS_DIR, "edition")

# Check that all categories names are unique
category_name_to_file = dict()
for file in categories_files:
    name = ae.get_field(file, "name")
    if name in category_name_to_file.keys():
        error(
            f"both '{file}' and '{category_name_to_file[name]}' provide category '{name}'"
        )
    category_name_to_file[name] = file
known_categories = set(category_name_to_file.keys())

# Check that all categories mentioned in components exist
component_name_to_file = dict()
for component_file in component_files:
    category = ae.get_field(component_file, "category")
    name = ae.get_field(component_file, "name")
    if category not in known_categories and category != "":
        error(
            f"'{category}' mentioned in component '{name}' was not found in {CATEGORIES_DIR}/"
        )
    component_name_to_file[name] = file
known_components = set(component_name_to_file.keys())

# Check that all categories mentioned in vendors components exist
vendors_component_name_to_file = dict()
for component_file in vendors_component_files:
    category = ae.get_field(component_file, "category")
    name = ae.get_field(component_file, "name")
    if category not in known_categories and category != "":
        error(
            f"'{category}' mentioned in vendors component '{name}' was not found in {CATEGORIES_DIR}/"
        )
    vendors_component_name_to_file[name] = file
known_vendors_components = set(vendors_component_name_to_file.keys())

# Check that all components mentioned in editions exist
for edition_file in edition_files:
    sections = ae.get_field(edition_file, "sections")
    for section in sections:
        components_pattern = "sections.%s.components" % section
        section_components = ae.get_field(edition_file, components_pattern)
        name = ae.get_field(edition_file, "name")

        section_components = set(section_components)
        if not section_components <= known_components:
            unknown_components = section_components.difference(known_components)
            error(
                f"'{name}' in section '{section}' consists unknown components:\n'{unknown_components}'"
            )
        vendors_components = section_components.intersection(known_vendors_components)
        if vendors_components:
            error(
                f"'{name}' in section '{section}' consists vendors components:\n'{vendors_components}'"
            )

# Check that all categories mentioned in other categories exist and detect loops
known_loops = set()
for category in known_categories:
    if category == "":
        continue

    cur_category = category
    visited = [cur_category]
    while cur_category:
        try:
            cur_category = ae.get_field(f"categories/{cur_category}/{cur_category}.category", "category")
        except FileNotFoundError:
            error(f"category '{cur_category}' is located in incorrectly named file")
            break
        except ae.FieldNotFoundError:
            break

        if cur_category not in known_categories:
            error(
                f"category '{cur_category}' mentioned in another category '{category}' was not found"
            )
            break

        if cur_category in visited:
            loop = visited[visited.index(cur_category):]
            if tuple(sorted(loop)) not in known_loops:
                known_loops.add(tuple(sorted(loop)))
                error(
                    f"categories loop detected: {loop}"
                )
            break

        visited.append(cur_category)

exit(exit_code)
