Summary:	OTP token authentication daemon
Name:		otpd
Version:	3.1.0
Release:	1
License:	GPL + Proprietary
Group:		System Environment/Daemons
Vendor:		TRI-D Systems, Inc.
URL:		http://www.tri-dsystems.com/
Packager:	TRI-D Systems, Inc. <support@tri-dsystems.com>
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

#Source:	http://www.tri-dsystems.com/software/dl/community/download.cgi?sw=%{name}-%{version}.tar.gz
Source:	%{name}-%{version}.tar.gz

BuildRequires: openssl-devel, openldap-devel
PreReq: /sbin/chkconfig /sbin/service

%description
otpd is part of a suite of software for authenticating users with
handheld OTP tokens.

%prep
%setup -q

%build
%configure
%{__make}

%install
%makeinstall
# install the init script
mkdir -p $RPM_BUILD_ROOT%{_initrddir}
%{__install} -m 0755 redhat/%{name}.init $RPM_BUILD_ROOT%{_initrddir}/%{name}
# create the /etc/sysconfig file
mkdir -p $RPM_BUILD_ROOT/etc/sysconfig
echo 'OPTIONS=""' > $RPM_BUILD_ROOT/etc/sysconfig/%{name}
chmod 0644 $RPM_BUILD_ROOT/etc/sysconfig/%{name}
# create the plugin socket directory
mkdir -p -m 0755 $RPM_BUILD_ROOT/var/run/%{name}
# schema
mkdir -p $RPM_BUILD_ROOT%{_libdir}/trid
%{__install} -m 0644 userops/otp.schema $RPM_BUILD_ROOT%{_libdir}/trid
%{__install} -m 0755 userops/schema2ad  $RPM_BUILD_ROOT%{_libdir}/trid

%clean
cd ..
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/%{buildsubdir}

%post
/sbin/chkconfig --add %{name}

%preun
# Uninstall
if [ $1 = 0 ]; then
  /sbin/service %{name} stop
  /sbin/chkconfig --del %{name}
fi

%postun
# Upgrade
if [ $1 -ge 1 ]; then
  /sbin/service %{name} restart
fi

%files
%defattr(-, root, root)
%doc NEWS README LICENSE.TRID README.API README.LICENSE
%dir /var/run/%{name}
%attr(0600, root, root) %config(noreplace) /etc/%{name}.conf
%config(noreplace) /etc/sysconfig/%{name}
%{_initrddir}/%{name}
%{_sbindir}/*
%{_libdir}/trid
%{_mandir}/*/*

%changelog
* Mon Feb 11 2008 Frank Cusack <frank@tri-dsystems.com> 3.1.0-1
- new release

* Thu Jan 31 2008 Frank Cusack <frank@tri-dsystems.com> 3.0.0-1
- new release

* Wed Dec 12 2007 Frank Cusack <frank@tri-dsystems.com> 2.5.2-1
- new release

* Tue Dec 11 2007 Frank Cusack <frank@tri-dsystems.com> 2.5.1-1
- new release

* Fri Dec 7 2007 Frank Cusack <frank@tri-dsystems.com> 2.5.0-1
- new release

* Fri Nov 9 2007 Frank Cusack <frank@tri-dsystems.com> 2.4.0-1
- new release
- add schema and otpauth

* Tue Oct 2 2007 Frank Cusack <frank@tri-dsystems.com> 2.3.0-1
- new release

* Thu Jul 12 2007 Frank Cusack <frank@tri-dsystems.com> 2.2.1-1
- new release 

* Tue May 22 2007 Frank Cusack <frank@tri-dsystems.com> 2.2.0-1
- new release

* Thu May 3 2007 Robert Flemming <flemming@tri-dsystems.com> 2.1.8-1
- New release

* Sat Apr 28 2007 Robert Flemming <flemming@tri-dsystems.com> 2.1.7-1
- Initial release
