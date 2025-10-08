%define _unpackaged_files_terminate_build 1

Name: alterator-application-control
Version: 0.1.0
Release: alt1

Summary: Alterator application for managing services
License: GPLv2+
Group: System/Configuration/Other
URL: https://altlinux.space/alterator/alt-control

Source0: %name-%version.tar

BuildRequires(pre): rpm-macros-cmake
BuildRequires(pre): rpm-macros-alterator
BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: qt6-base-devel
BuildRequires: qt6-tools-devel
BuildRequires: qt6-base-common

Requires: alterator-interface-control
Requires: alterator-manager >= 0.1.25
Requires: alterator-module-executor >= 0.1.14

%description
%summary.

%prep
%setup

%build
%cmake
%cmake_build

%install
%cmakeinstall_std


mkdir -p %buildroot%_alterator_datadir/backends
mkdir -p %buildroot%_alterator_datadir/applications
install -v -p -m 644 -D alterator/alterator-application-control.backend %buildroot%_alterator_datadir/backends
install -v -p -m 644 -D alterator/alterator-application-control.application %buildroot%_alterator_datadir/applications

%files
%_bindir/alt-control
%doc *.md
%dir %_alterator_datadir/backends
%dir %_alterator_datadir/applications
%_alterator_datadir/backends/*.backend
%_alterator_datadir/applications/*.application

%changelog
* Wed Mar 19 2025 Andrey Alekseev <parovoz@altlinux.org> 0.1.0-alt1
- initial build
