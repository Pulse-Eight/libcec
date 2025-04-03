### Debian
Use the following commands to create a Debian package:
```
sudo apt-get build-dep . # to install build dependencies
sed "s/#DIST#/$(lsb_release -cs)/g" debian/changelog.in > debian/changelog
dpkg-buildpackage --no-sign # omit this argument if you have a private key to sign with
```
