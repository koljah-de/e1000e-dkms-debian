# Intel e1000e ethernet adapter driver (dkms version) for Debian

Intel® Network Adapter Driver for PCIe* Intel® Gigabit Ethernet Network Connections Under Linux* 

This is a Debian dkms package version of the latest code of Intels e1000e ethernet driver available from https://sourceforge.net/projects/e1000/

---

dep: dkms

You should have installed: linux-headers-amd64 dkms build-essential 

---

To install the deb package run:
* dpkg -i e1000e-dkms_<x.x.x.x>_all.deb

To remove the driver run:
* apt purge e1000e-dkms

To build a deb package from source run:
* dpkg-deb --build e1000e-dkms

---

If you want to use the dkms kernel module only (works with all Linux distributions) run:
* cp -r e1000e-dkms/usr/src/e1000e-<x.x.x.x> /usr/local/src/
* dkms add /usr/local/src/e1000e-<x.x.x.x>
* dkms build /usr/local/src/e1000e-<x.x.x.x>
* dkms install /usr/local/src/e1000e-<x.x.x.x>

To remove the dkms kernel module only (works with all Linux distributions) run:
* dkms remove /usr/local/src/e1000e-<x.x.x.x> --all

---

For further information visit:
* https://www.intel.com/content/www/us/en/support/articles/000005480/network-and-i-o/ethernet-products.html
* https://downloadcenter.intel.com/download/15817
* https://sourceforge.net/projects/e1000/
