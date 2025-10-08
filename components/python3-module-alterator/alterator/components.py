#!/usr/bin/python3

import dbus
import os
import sys
import tomllib
from typing import List, Dict, Optional

class Component:
    """Component class"""
    name = None
    display_name = None
    comment = None
    category = None
    packages = []
    _installed = False

    def __init__(self, name: str, display_name: str, category: str, comment: str, packages: str):
        self.name = name
        self.display_name = display_name
        self.category = category
        self.comment = comment
        self.packages = packages

    def set_installed(self, value: bool):
        self._installed = value
        # TODO

    def get_installed(self) -> bool:
        self._installed = Components().installed(self.name)
        return self._installed

    installed = property(get_installed, set_installed)

class Components:
    """Component list"""
    bus = None
    lang = None

    def get_system_locale(self) -> str:
        """Detect system locale"""
        if self.lang:
            return self.lang
        for var in ['LC_ALL', 'LANG', 'LANGUAGE']:
            locale = os.getenv(var, '')
            if locale:
                self.lang = locale.split('.')[0].split('_')[0].lower()
                return self.lang
        return 'en'

    def get_localized_name(self, component: Dict) -> str:
        """Get localized name"""
        display_names = component.get('display_name', {})
        if not isinstance(display_names, dict):
            return component.get('name', 'Unnamed Component')
 
        locale = self.get_system_locale()
        for lang in [locale, locale.split('_')[0], 'en']:
            if lang in display_names:
                return display_names[lang]

    def get_localized_comment(self, component: Dict) -> str:
        """Get localized comment"""
        comments = component.get('comment', {})
        if not isinstance(comments, dict):
            return component.get('name', 'Unnamed Component')
 
        locale = self.get_system_locale()
        for lang in [locale, locale.split('_')[0], 'en']:
            if lang in comments:
                return comments[lang]

    def get_dbus(self):
        """Get system dbus"""
        if not self.bus:
            try:
                self.bus = dbus.SystemBus()
            except dbus.exceptions.DBusException as e:
                print(f"Unable to connect to dbus system bus. {e}", file=sys.stderr)
        return self.bus

    def fetch(self) -> List[Component]:
        """Get complete list of components"""
        bus = self.get_dbus() 
        try:
            proxy = bus.get_object('org.altlinux.alterator', '/org/altlinux/alterator/global')
            interface = dbus.Interface(proxy, 'org.altlinux.alterator.batch_components1')
            raw_data, _ = interface.Info()

            components = []
            toml_strings = [s.decode('utf-8') for s in bytes(raw_data).split(b'\x00') if s]

            for toml_str in toml_strings:
                try:
                    data = tomllib.loads(toml_str)
                    components.append(data)
                except tomllib.TOMLDecodeError:
                    continue

            result = []
            for comp in components:
                if not comp.get('name'):
                    continue

                result.append(Component(
                    comp['name'],
                    self.get_localized_name(comp),
                    comp.get('category', 'unknown'),
                    self.get_localized_comment(comp),
                    list(comp.get('packages', {}).keys())
                    ))

            return result

        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            return []

    def installed(self, name: str) -> bool:
        """Check if component installed"""
        if not name:
            return False
        bus = self.get_dbus() 
        comp_name = name.replace('-', '_')
        comp_path = f'/org/altlinux/alterator/component_{comp_name}'
        installed = False
        try:
            comp_proxy = bus.get_object('org.altlinux.alterator', comp_path)
            comp_interface = dbus.Interface(comp_proxy, 'org.altlinux.alterator.component1')
            _, status = comp_interface.Status()
            installed = not bool(status)
        except dbus.exceptions.DBusException:
            pass
        return installed

    def description(self, name: str) -> str:
        """Get component description"""
        if not name:
            return ""
        bus = self.get_dbus() 
        comp_name = name.replace('-', '_')
        comp_path = f'/org/altlinux/alterator/component_{comp_name}'
        description = False
        try:
            comp_proxy = bus.get_object('org.altlinux.alterator', comp_path)
            comp_interface = dbus.Interface(comp_proxy, 'org.altlinux.alterator.component1')
            raw_data, _ = comp_interface.Description()
            strings = [s.decode('utf-8') for s in bytes(raw_data).split(b'\x00') if s]
            return "\n".join(strings)
        except dbus.exceptions.DBusException:
            pass
        return description
