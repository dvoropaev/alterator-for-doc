import os
import sys
import json
from typing import Any, Union, Optional, List
from dataclasses import dataclass
from pathlib import Path
import tomlkit

# Lazy imports:
# import jsonschema


@dataclass
class FieldNotFoundError(Exception):
    pattern: str
    key: str


class UnsupportedValueTypeError(Exception):
    """Raised when field contains unsupported value types (tables or nested lists)"""

    pass


class ValidationError(Exception):
    """Base exception for all validation errors"""

    pass


def is_simple_value(item: Any) -> bool:
    """Check if an item is a simple value type."""
    return isinstance(item, (str, int, float, bool))


def is_table_value(item: Any) -> bool:
    """Check if an item is a table value type."""
    return isinstance(item, (tomlkit.items.Table))


def get_field(file_path: Union[str, Path], pattern: str) -> Any:
    """
    Retrieve field values from a TOML file based on a dot-separated pattern.
    Returns a list of simple values (strings, numbers, booleans).
    Returns key names for tables.

    Args:
        file_path: Path to the TOML file
        pattern: Dot-separated field pattern (e.g. "server.ports")

    Raises:
        FieldNotFoundError: If specified field pattern is not found
        UnsupportedValueTypeError: If field contains unsupported value types
    """
    with open(Path(file_path), "rb") as f:
        data = tomlkit.load(f)

    # Navigate through the nested structure
    for key in pattern.split("."):
        if not isinstance(data, dict) or key not in data:
            raise FieldNotFoundError(pattern=pattern, key=key)
        data = data[key]

    if is_simple_value(data):
        return data

    if is_table_value(data):
        return list(data)

    if not isinstance(data, list) or not all(is_simple_value(item) for item in data):
        raise UnsupportedValueTypeError(
            "Only simple values are supported (strings, numbers, booleans)"
        )

    return data


def validate(
    file_path: Union[str, Path], schemas_dir: Union[str, Path] | None = None
) -> None:
    """
    Validate a TOML file against an Alterator Entry schema based on its 'type' field.

    Args:
        file_path: Path to TOML file to validate

    Raises:
        ValidationError: If validation fails for any reason
        toml.TomlDecodeError: If TOML file cannot be parsed
    """
    import jsonschema

    schemas_dir = Path(
        schemas_dir
        or os.getenv("ALTERATOR_SCHEMAS_DIR", "/usr/share/alterator/schemas")
    )

    with open(Path(file_path), "rb") as f:
        data = tomlkit.load(f)

    if "type" not in data:
        raise ValidationError("Configuration missing required 'type' field")

    # Load and parse schema
    schema_path = Path(schemas_dir) / f"{str(data['type'].value).lower()}.schema.json"

    try:
        schema = json.loads(schema_path.read_text())
    except (IOError, json.JSONDecodeError) as e:
        raise ValidationError(
            f'Cannot load schema "{schema_path}" for type "{data["type"]}": {str(e)}'
        )

    # Validate against schema
    try:
        jsonschema.validate(instance=data, schema=schema)
    except jsonschema.ValidationError as e:
        raise ValidationError(f"Schema validation failed: {str(e)}")


@dataclass
class PackagesFilterOptions:
    kflavours: List[str]
    arch: str
    section: Optional[str]
    desktop: Optional[str]
    language: str
    image_ignore: bool


def extract_packages_from_component(
    packages: dict, given_options: PackagesFilterOptions
) -> list[str]:
    """
    Filter and return package names from a packages dict using the given options.
    """
    needed_packages = []

    for package_name, pkg_options in packages.items():
        if given_options.image_ignore and pkg_options.get("image_ignore", False):
            continue

        # Check architecture:
        arch = pkg_options.get("arch")
        exclude_arch = pkg_options.get("exclude_arch")
        if not (arch is None or given_options.arch in arch):
            continue
        if exclude_arch is not None and given_options.arch in exclude_arch:
            continue

        # Check desktop:
        pkg_desktop = pkg_options.get("desktop")
        if not (
            given_options.desktop is None
            or pkg_desktop is None
            or given_options.desktop == pkg_desktop
        ):
            continue

        # Check language:
        pkg_language = pkg_options.get("language")
        if not (pkg_language is None or given_options.language == pkg_language):
            continue

        if pkg_options.get("kernel_module"):
            for kf in given_options.kflavours:
                needed_packages.append(f"{package_name}-{kf}")
        else:
            needed_packages.append(package_name)

    return needed_packages


def extract_packages_from_edition(
    edition_file: Union[str, Path],
    given_options: PackagesFilterOptions,
    components_dir: str = "/usr/share/alterator/components",
) -> set[str]:
    """
    Extract package names from given edition by processing components files.
    """
    with open(edition_file, "rb") as f:
        edition_data = tomlkit.load(f)

    sections = edition_data["sections"].value
    assert sections

    components = []
    for section_name in sections:
        if given_options.section is not None and section_name != given_options.section:
            continue
        section = sections[section_name]
        section_components = section["components"]
        components.extend(section_components)

    pkgs = set()
    for component_name in components:
        component_name = component_name.strip()
        component_file = f"{components_dir}/{component_name}/{component_name}.component"
        try:
            with open(component_file, "rb") as f:
                component_data = tomlkit.load(f)
        except FileNotFoundError as e:
            print(
                f'WARNING: component "{component_name}" not found: {e}',
                file=sys.stderr,
            )
            continue

        component_pkgs = extract_packages_from_component(
            component_data["packages"].value,
            PackagesFilterOptions(
                kflavours=given_options.kflavours,
                arch=given_options.arch,
                section=None,
                desktop=given_options.desktop,
                language=given_options.language,
                image_ignore=True,
            ),
        )
        pkgs.update(component_pkgs)

    return pkgs
