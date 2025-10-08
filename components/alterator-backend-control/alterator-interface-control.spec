%define _unpackaged_files_terminate_build 1

Name: alterator-backend-control
Version: 0.1.0
Release: alt1

Summary: Alterator interface for system restrictions management
License: GPLv2+
Group: Other
URL: https://gitlab.basealt.space/alt/altrerator-backend-control

BuildArch: noarch

BuildRequires(Pre): rpm-macros-alterator

Source0: %name-%version.tar

%description
Alterator interface for system restrictions management.

%package -n alterator-interface-control
Summary: XML files for org.altlinux.alterator.control1 interface
Group: System/Configuration/Other
Version: 0.1.0
Release: alt1

%description -n alterator-interface-control
XML files describing D-Bus interface org.altlinux.alterator.control1.


%prep
%setup

%install

mkdir -p %buildroot%_datadir/dbus-1/interfaces
mkdir -p %buildroot%_datadir/polkit-1/actions

install -p -m 644 -D org.altlinux.alterator.control1.xml %buildroot%_datadir/dbus-1/interfaces/org.altlinux.alterator.control1.xml
install -p -m 644 -D org.altlinux.alterator.control1.policy %buildroot%_datadir/polkit-1/actions/org.altlinux.alterator.control1.policy

mkdir -p %buildroot%_alterator_datadir/backends
mkdir -p %buildroot%_alterator_datadir/objects

install -v -p -m 644 -D control.object %buildroot%_alterator_datadir/objects
install -v -p -m 644 -D control.backend %buildroot%_alterator_datadir/backends
install -v -p -m 755 -D control-helper %buildroot%_libexecdir/%name/control-helper

%files
%dir %_alterator_datadir/objects
%_alterator_datadir/objects/control.object
%dir %_alterator_datadir/backends
%_alterator_datadir/backends/control.backend
%_libexecdir/%name/control-helper

%files -n alterator-interface-control
%_datadir/dbus-1/interfaces/org.altlinux.alterator.control1.xml
%_datadir/polkit-1/actions/org.altlinux.alterator.control1.policy

%changelog
* Tue Mar 18 2025 Andrey Alekseev <parovoz@altlinux.org> 0.1.0-alt1
- initial build
