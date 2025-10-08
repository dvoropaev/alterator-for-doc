**alt-components-base**

[English](README.md) | [Русский](./docs/README.ru_RU.md)

Designed for building hierarchical structure of system components.

---

# Overview
The project uses 3 alterator-entry entities:
| Entity  |    Format   | Description   |
| ----------| ------------| -----------|
| Component | `.component`| A set of logically grouped RPM packages|
| Category | `.category` | Entity for grouping components by thematic criteria|
| Edition  | `.edition`  | Top-level entity designed for creating target distribution configurations|

Entities are described in TOML format and must strictly comply with the specification documented in the [alterator-entry](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc) project. The project also provides [examples of component definitions.](https://altlinux.space/alterator/alterator-entry/src/branch/master/examples/component)

Links to entity specifications::
- [Component](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc#%D1%81%D1%83%D1%89%D0%BD%D0%BE%D1%81%D1%82%D1%8C-%D1%82%D0%B8%D0%BF%D0%B0-component)
- [Category](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc#%D1%81%D1%83%D1%89%D0%BD%D0%BE%D1%81%D1%82%D1%8C-%D1%82%D0%B8%D0%BF%D0%B0-category)
- [Edition](https://altlinux.space/alterator/alterator-entry/src/branch/master/doc#%D1%81%D1%83%D1%89%D0%BD%D0%BE%D1%81%D1%82%D1%8C-%D1%82%D0%B8%D0%BF%D0%B0-edition)

# Client Interaction
Communication between components (categories and sections) and clients (such as the graphical application [alt-components](https://altlinux.space/alterator/alt-components) or the components module of the [alteratorctl](https://altlinux.space/alterator/alteratorctl) utility) is organized through D-Bus - an inter-process communication system.

However, the alt-components-base project does not directly provide the interaction interface; this is handled by the [alterator-backend-component](https://altlinux.space/alterator/alterator-backend-component) package:

1. The `alterator-backend-component` сpackage contains a filetrigger that, when new components are added to the system (located in `/usr/share/alterator/components`) executes the script `/usr/lib/alterator/backends/component.d/generate-components-backends`, which generates backend files required by [alterator-manager](https://altlinux.space/alterator/alterator-manager).

2. `alterator-manager` reads the new backend files and generates objects `/org/altlinux/alterator/<component_name>` on D-Bus, implementing the `org.altlinux.alterator.component1` interface.

Thus, component developers don't need to worry about creating objects and implementing interfaces - it's sufficient to add the necessary files (`.component`, `description.md` и т.д.) to alt-components-base.

> For optimized interaction with components and categories over D-Bus, the object `/org/altlinux/alterator/global`, is provided, offering interfaces for working with them (`org.altlinux.alterator.batch_component_categories1`, `org.altlinux.alterator.batch_components1`, `org.altlinux.alterator.component_categories1`, `org.altlinux.alterator.current_edition1`)

> The listed interfaces are provided by the `alterator-backend-component` package

# Development Workflow
1. Fork the repository.
2. Add entity files (.component, .category, .edition) to the project.
3. Validate your changes (run `./scripts/validate_categories.py`).
4. Submit your contribution.

> Before submitting a PR, it's recommended to test your changes in a test environment (using the `alt-components` and `alteratorctl components` clients)
