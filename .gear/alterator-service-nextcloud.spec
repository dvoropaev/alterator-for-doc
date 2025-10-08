%define _unpackaged_files_terminate_build 1
%define service service-nextcloud
Name: alterator-service-nextcloud
Version: 0.1
Release: alt1

Summary: Service for managment nextcloud
License: GPLv3
Group: System/Configuration/Other
URL: https://gitlab.basealt.space/alt/alterator-service-nextcloud

BuildArch: noarch
Source: %name-%version.tar

BuildRequires(pre): rpm-macros-alterator

Requires: alterator-module-executor
Requires: alterator-interface-service
Requires: alterator-entry

%description
Service for deploy nextcloud.

%prep
%setup

%install
mkdir -p %buildroot%_alterator_datadir/service

install -p -D -m644 alterator/%service.backend %buildroot%_alterator_datadir/backends/%service.backend
install -p -D -m644 alterator/%service.service %buildroot%_alterator_datadir/service/%service.service

%files
%_alterator_datadir/backends/%service.backend
%_alterator_datadir/service/%service.service

%changelog
* Tue Jul 15 2025 Evgenii Sozonov <arzdez@altlinux.org> 0.1-alt1
- Initial commit
