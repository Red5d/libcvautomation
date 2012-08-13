#Specfile for Libcvautomation
#This specfile builds the library and headers for Libcvautomation
%global VERSION 1.5
%global PATCHLEVEL 1

Summary: GUI Automation and Testing tool based on OpenCV and XTest
Name: libcvautomation
Version: %{VERSION}
Release: %{PATCHLEVEL}
Source: https://github.com/DjBushido/libcvautomation/%{name}-%{VERSION}.tar.gz
URL: http://djbushido.github.com/libcvautomation/
Vendor: MOSAIC at University of North Carolina at Charlotte
Packager: @PACKAGER <bspeice@uncc.edu>
License: BSD 2-clause
Group: Development/Libraries

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: libXtst-devel
BuildRequires: libX11-devel
BuildRequires: opencv-devel
BuildRequires: doxygen

%description
Libcvautomation is a GUI automation and testing tool based on image
recognition and response. This program was designed as a direct replacement
for Sikuli and Xpresser. Each solution had large problems with crashing, and
both refused to function on Red Hat Linux and Ubuntu 12.04. The author really
liked the way each of these programs approached GUI automation, but they simply
didn't work. As such, a simple library was designed to integrate OpenCV and
XTest, which can be used by BASH to drive GUI testing and automation, and works
on both new and old Linux distributions.

%package devel
Summary: Library links and header files for libcvautomation application development
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: libXtst-devel
Requires: libX11-devel
Requires: opencv-devel

%description devel
libcvautomation-devel contains all files needed to build programs
on top of libcvautomation.

%package examples
Summary: Example programs to demonstrate libcvautomation functionality
Group: Development/Tools
Requires: %{name} = %{version}-%{release}

%description examples
libcvautomation-examples contains programs designed to showcase the
functionality of libcvautomation.

%package doc
Summary: HTML documentation for libcvautomation
Group: Documentation
Requires: %{name} = %{version}-%{release}

%description doc
libcvautomation-doc contains the HTML documentation for libcvautomation.

%prep
%setup -q

%build
%configure \
	--disable-static
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%postun -p /sbin/ldconfig

%files
%attr(755, root, root) /usr/lib64/*.la
%attr(755, root, root) /usr/lib64/*.so*
%attr(644, root, root) /usr/share/man/man3/*

%files devel
%attr(644, root, root) /usr/lib64/pkgconfig/*
%attr(644, root, root) /usr/include/libcvautomation/*

%files examples
%attr(644, root, root) /usr/share/man/man1/*
%attr(755, root, root) /usr/bin/*
%attr(755, root, root) /etc/*

%files doc
%attr(644, root, root) /usr/share/libcvautomation/html/*
%attr(755, root, root) /usr/share/libcvautomation/html/installdox

%changelog
* Mon Jul 30 2012 Bradlee Speice <bspeice@uncc.edu> 1.4-1
- Release version 1.4 of libcvautomation

* Mon Aug 13 2012 Bradlee Speice <bspeice@uncc.edu> 1.5-1
- Release version 1.5 of libcvautomation
