%define _unpackaged_files_terminate_build 1

Name: package1
Version: 0.2.10
Release: alt1

Summary: Lorem ipsum dolor sit amet, consectetur adipiscing elit
License: GPLv2+
Group: System/Configuration/Other
URL: https://altlinux.space/alterator/alterator-backend-packages
Requires: package2 package3 package4 package5

BuildArch: noarch
Source0: %name-%version.tar

%description
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum
dictum semper libero, non cursus dui mattis a. Suspendisse id libero
imperdiet, lacinia enim eget, bibendum ex. Fusce imperdiet iaculis
eleifend. Nam ac aliquam dolor. Etiam efficitur ullamcorper diam, vitae
fringilla ligula laoreet nec. Vivamus non odio consectetur, mollis erat
nec, rutrum velit. Integer vel sagittis sapien. Aenean aliquam ornare
neque. Curabitur in risus commodo, pharetra purus ac, posuere magna.

%package -n package2
Summary: Alterator interfaces for managing system packages
Group: System/Configuration/Other
Requires: package4

%description -n package2
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum
dictum semper libero, non cursus dui mattis a. Suspendisse id libero
imperdiet, lacinia enim eget, bibendum ex. Fusce imperdiet iaculis
eleifend. Nam ac aliquam dolor. Etiam efficitur ullamcorper diam, vitae
fringilla ligula laoreet nec. Vivamus non odio consectetur, mollis erat
nec, rutrum velit. Integer vel sagittis sapien. Aenean aliquam ornare
neque. Curabitur in risus commodo, pharetra purus ac, posuere magna.

%package -n package3
Summary: Alterator interfaces for managing system packages
Group: System/Configuration/Other

%description -n package3
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum
dictum semper libero, non cursus dui mattis a. Suspendisse id libero
imperdiet, lacinia enim eget, bibendum ex. Fusce imperdiet iaculis
eleifend. Nam ac aliquam dolor. Etiam efficitur ullamcorper diam, vitae
fringilla ligula laoreet nec. Vivamus non odio consectetur, mollis erat
nec, rutrum velit. Integer vel sagittis sapien. Aenean aliquam ornare
neque. Curabitur in risus commodo, pharetra purus ac, posuere magna.

%package -n package4
Summary: Alterator interfaces for managing system packages
Group: System/Configuration/Other

%description -n package4
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum
dictum semper libero, non cursus dui mattis a. Suspendisse id libero
imperdiet, lacinia enim eget, bibendum ex. Fusce imperdiet iaculis
eleifend. Nam ac aliquam dolor. Etiam efficitur ullamcorper diam, vitae
fringilla ligula laoreet nec. Vivamus non odio consectetur, mollis erat
nec, rutrum velit. Integer vel sagittis sapien. Aenean aliquam ornare
neque. Curabitur in risus commodo, pharetra purus ac, posuere magna.

%package -n package5
Summary: Alterator interfaces for managing system packages
Group: System/Configuration/Other

%description -n package5
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum
dictum semper libero, non cursus dui mattis a. Suspendisse id libero
imperdiet, lacinia enim eget, bibendum ex. Fusce imperdiet iaculis
eleifend. Nam ac aliquam dolor. Etiam efficitur ullamcorper diam, vitae
fringilla ligula laoreet nec. Vivamus non odio consectetur, mollis erat
nec, rutrum velit. Integer vel sagittis sapien. Aenean aliquam ornare
neque. Curabitur in risus commodo, pharetra purus ac, posuere magna.

%package -n package6
Summary: Alterator interfaces for managing system packages
Group: System/Configuration/Other
Conflicts: package1 package2 package3 package4 package5

%description -n package6
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum
dictum semper libero, non cursus dui mattis a. Suspendisse id libero
imperdiet, lacinia enim eget, bibendum ex. Fusce imperdiet iaculis
eleifend. Nam ac aliquam dolor. Etiam efficitur ullamcorper diam, vitae
fringilla ligula laoreet nec. Vivamus non odio consectetur, mollis erat
nec, rutrum velit. Integer vel sagittis sapien. Aenean aliquam ornare
neque. Curabitur in risus commodo, pharetra purus ac, posuere magna.

%install
mkdir -p %buildroot%_bindir/

echo > %buildroot%_bindir/helloworld1 <<-EOF
#!/bin/bash
echo "hello world"
	EOF

chmod +x %buildroot%_bindir/helloworld1
cp %buildroot%_bindir/helloworld1 %buildroot%_bindir/helloworld2
cp %buildroot%_bindir/helloworld1 %buildroot%_bindir/helloworld3
cp %buildroot%_bindir/helloworld1 %buildroot%_bindir/helloworld4
cp %buildroot%_bindir/helloworld1 %buildroot%_bindir/helloworld5
cp %buildroot%_bindir/helloworld1 %buildroot%_bindir/helloworld6

%files
%_bindir/helloworld1

%files -n package2
%_bindir/helloworld2

%files -n package3
%_bindir/helloworld3

%files -n package4
%_bindir/helloworld4

%files -n package5
%_bindir/helloworld5

%files -n package6
%_bindir/helloworld6

%changelog