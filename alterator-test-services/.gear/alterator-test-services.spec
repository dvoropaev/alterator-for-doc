%define _unpackaged_files_terminate_build 1
Name: alterator-test-services
Version: 0.1.2
Release: alt1

Summary: Test services for alterator
License: GPLv3
Group: System/Configuration/Other
URL: https://gitlab.basealt.space/alt/alterator-test-services

BuildArch: noarch

Source: %name-%version.tar

BuildRequires(pre): rpm-macros-alterator

BuildRequires: jq
Requires: alterator-module-executor >= 0.1.14
Requires: alterator-interface-service >= 0.2.1
Requires: alterator-entry


%description
Test services for alterator.

%prep
%setup

%install
mkdir -p %buildroot%_bindir
install -p -D -m755 test-services-helper %buildroot%_bindir/test-services-helper

mkdir -p %buildroot%_alterator_datadir/services
mkdir -p %buildroot%_alterator_datadir/backends
mkdir -p %buildroot%_libexecdir/test-services/
install -p -D -m755 diag-helper %buildroot%_libexecdir/test-services/diag-helper

for i in test_service*
do
    install -p -D -m644 $i/$i.service %buildroot%_alterator_datadir/services/$i.service

    install -p -D -m644 test-service.backend.template %buildroot%_alterator_datadir/backends/$i.backend
    sed -i "s/<SERVICE>/$i/g" %buildroot%_alterator_datadir/backends/$i.backend

    if [ -f $i/tests ]
    then
        install -p -D -m644 $i/$i.diag    %buildroot%_alterator_datadir/test_service_diag/$i.diag
        install -p -D -m644 test-service-diag.backend.template %buildroot%_alterator_datadir/backends/$i-diag.backend
        sed -i "s/<SERVICE>/$i/g" %buildroot%_alterator_datadir/backends/$i-diag.backend
        grep parameters $i/$i.diag | sed 's/parameters/methods.Run.environment/g' >> %buildroot%_alterator_datadir/backends/$i-diag.backend
        install -p -D -m644 $i/tests %buildroot%_libexecdir/test-services/$i/tests
    fi

    install -p -D -m644 $i/params_example.json %buildroot%_libexecdir/test-services/$i/params_example.json

    ./schema-generator $i/$i.service %buildroot%_libexecdir/test-services/$i/
done


%files
%_bindir/test-services-helper
%_libexecdir/test-services/diag-helper
%dir %_libexecdir/test-services
%dir %_libexecdir/test-services/test_service*
%_libexecdir/test-services/*/*
%dir %_alterator_datadir/test_service_diag
%_alterator_datadir/test_service_diag/*
%_alterator_datadir/services/test_*.service
%_alterator_datadir/backends/test_*.backend


%changelog
* Thu Mar 13 2025 Andrey Alekseev <parovoz@altlinux.org> 0.1.2-alt1
- update to latest dbus interface changes
- use schema generator

* Tue Mar 11 2025 Aleksey Saprunov <sav@altlinux.org> 0.1.1-alt1
- Add CHANGELOG.md

* Thu Mar 06 2025 Andrey Alekseev <parovoz@altlinux.org> 0.1-alt1
- Initial commit
