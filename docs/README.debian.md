### Debian
Use the following commands to create a Debian package:
```
source /etc/lsb-release
sed "s/#DIST#/${DISTRIB_CODENAME}/g" debian/changelog.in > debian/changelog
dpkg-buildpackage
```
