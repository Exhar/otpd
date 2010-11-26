Summary:	OTP token authentication daemon
Name:		otpd
Version:	3.2.7
Release:	1%{?dist}
License:	GPLv2+
Group:		System Environment/Daemons
URL:		http://otpd.googlecode.com/
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

Source:		http://otpd.googlecode.com/files/%{name}-%{version}.tar.gz

BuildRequires:  openssl-devel, openldap-devel
PreReq: /sbin/chkconfig /sbin/service

%description
OTPD is an authentication server capable of validating OTP tokens
using the HOTP standard (RFC 4226).

This software can be used in conjunction with a RADIUS server,
such as FreeRADIUS, to authenticate users.

%prep
%setup -q

%build
%configure
%{__make} %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT

# install the init script
mkdir -p $RPM_BUILD_ROOT%{_initrddir}
%{__install} -m 0755 redhat/%{name}.init $RPM_BUILD_ROOT%{_initrddir}/%{name}
# create the /etc/sysconfig file
mkdir -p $RPM_BUILD_ROOT/etc/sysconfig
echo 'OPTIONS=""' > $RPM_BUILD_ROOT/etc/sysconfig/%{name}
chmod 0644 $RPM_BUILD_ROOT/etc/sysconfig/%{name}
# Create the state directory
mkdir -p $RPM_BUILD_ROOT/etc/otpstate
chmod 0700 $RPM_BUILD_ROOT/etc/otpstate
# Create an empty otppasswd file
touch $RPM_BUILD_ROOT/etc/otppasswd
chmod 0600 $RPM_BUILD_ROOT/etc/otppasswd
# create the plugin socket directory
mkdir -p -m 0755 $RPM_BUILD_ROOT/var/run/%{name}
# schema
mkdir -p $RPM_BUILD_ROOT%{_libdir}/otpd
%{__install} -m 0644 userops/otp.schema $RPM_BUILD_ROOT%{_libdir}/otpd
%{__install} -m 0755 userops/schema2ad  $RPM_BUILD_ROOT%{_libdir}/otpd

%clean
rm -rf $RPM_BUILD_ROOT

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
%doc NEWS README README.API README.LICENSE
%dir /var/run/%{name}
%dir /etc/otpstate/
%attr(0600, root, root) %config(noreplace) /etc/%{name}.conf
%attr(0600, root, root) %config(noreplace) /etc/otppasswd
%config(noreplace) /etc/sysconfig/%{name}
%{_initrddir}/%{name}
%{_sbindir}/*
%{_libdir}/otpd
%{_mandir}/*/*

%changelog
* Thu Nov 20 2009 Giuseppe Paterno' <gpaterno@gpaterno.com> 3.2.5-2
- Changed resynctool to allow debug output. Fixed also man pages.

* Thu Nov 20 2009 Giuseppe Paterno' <gpaterno@gpaterno.com> 3.2.4-1
- Fixed man pages installation and creation of empty otppasswd

* Thu Nov 20 2009 Giuseppe Paterno' <gpaterno@gpaterno.com> 3.2.3-1
- Fixed 32-bit/64-bit issue in bitmanip.h

* Thu Nov 20 2009 Giuseppe Paterno' <gpaterno@gpaterno.com> 3.2.2-1
- Fixed man pages to reflect the fork

* Thu Nov 19 2009 Giuseppe Paterno' <gpaterno@gpaterno.com> 3.2.1-1
- Cleaned up some code (failover_thread.c and resynctool.c) to comply with strict compiling

* Mon Nov 16 2009 Giuseppe Paterno' <gpaterno@gpaterno.com> 3.2.1-1
- Polished for the EPEL repository, first release in fedoraproject

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
