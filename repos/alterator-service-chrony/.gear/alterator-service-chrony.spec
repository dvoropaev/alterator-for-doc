%define _unpackaged_files_terminate_build 1
%define service service-chrony
Name: alterator-service-chrony
Version: 0.1.2
Release: alt1

Summary: Service for managment chrony
License: GPLv3
Group: System/Configuration/Other
URL: https://altlinux.space/alterator/alterator-service-chrony

BuildArch: noarch
Source: %name-%version.tar

BuildRequires(pre): rpm-macros-alterator

Requires: alterator-module-executor
Requires: alterator-interface-service
Requires: alterator-entry

%description
Service for deploy chrony.

%prep
%setup

%install
mkdir -p %buildroot%_alterator_datadir/services
mkdir -p %buildroot%_localstatedir/alterator/service/%service/backups

install -p -D -m755 %service %buildroot%_bindir/%service
install -p -D -m644 %service.backend %buildroot%_alterator_datadir/backends/%service.backend
install -p -D -m644 %service.service %buildroot%_alterator_datadir/services/%service.service

%files
%_alterator_datadir/backends/%service.backend
%_alterator_datadir/services/%service.service
%_bindir/%service
%_localstatedir/alterator/service/%service/backups

%changelog
* Thu Jul 31 2025 Evgenii Sozonov <arzdez@altlinux.org> 0.1.2-alt1
- Backup directory has been changed, undeploy function has been fixed (thx Mukhin Michael)

* Tue Jul 15 2025 Evgenii Sozonov <arzdez@altlinux.org> 0.1-alt1
- Initial commit