# Intel e1000e ethernet adapter driver (DKMS version) for Debian

__Intel® Network Adapter Driver for PCIe* Intel® Gigabit Ethernet Network Connections Under Linux*__

This is a Debian DKMS package version of the latest code of Intels e1000e ethernet driver available from https://sourceforge.net/projects/e1000/

---

## Prerequisites
**Dependency:** dkms

You should have installed: linux-headers dkms build-essential
```
apt install linux-headers-$(uname -r) dkms build-essential
```

---

## Install and build the deb package

To install the deb package run:
```
dpkg -i e1000e-dkms_<x.x.x.x>_all.deb
```

To remove the driver run:
```
apt purge e1000e-dkms
```

To build a deb package from source run:
```
dpkg-deb --build e1000e-dkms
```

---

## Install and build the dkms kernel module only

If you want to use the dkms kernel module only (works with all Linux distributions) run:
```
cp -r e1000e-dkms/usr/src/e1000e-<x.x.x.x> /usr/src/
dkms add -m e1000e -v <x.x.x.x>
dkms build -m e1000e -v <x.x.x.x>
dkms install -m e1000e -v <x.x.x.x>
```

To remove the dkms kernel module only (works with all Linux distributions) run:
```
dkms remove -m e1000e -v <x.x.x.x> --all
```

---

## Official Intel websites regarding the e1000e ethernet driver

For further information visit:
* https://www.intel.com/content/www/us/en/support/articles/000005480/network-and-i-o/ethernet-products.html
* https://downloadcenter.intel.com/download/15817
* https://sourceforge.net/projects/e1000/

---

## Changelog

**Changelog for 3.4.2.3**
* Fix build on newer kernels (4.18+)
* Backport to upstream: Backport to upstream: 0bcd952fee (ethernet/intel: consolidate NAPI and NAPI exit)
* Minor bug fixes

**Changelog for 3.4.2.1**
* Fix build on newer kernels (4.15+)
* Backport to upstream: 91c527a556 ("ethernet/intel: use core min/max MTU checking")

**Changelog for 3.4.1.1**
* Fix compilation error on Red Hat 7.5

**Changelog for 3.4.0.2**
* Fix Overflow Buffer.
* Initial support for the following devices:
  * Ethernet Connection (6) I219-LM
  * Ethernet Connection (6) I219-V
  * Ethernet Connection (7) I219-LM
  * Ethernet Connection (7) I219-V  
* Minor bug fixes.
* Cosmetic changes.

**Changelog for 3.3.6**
* Fix for Tx Hang:
  * I219LM and I219V devices can fall into unrecovered Tx hang under very stressfully UDP traffic and multiple reconnection of Ethernet cable. This Tx hang of the LAN Controller is only recovered if the system is rebooted.
  * More information: https://www.intel.com/content/dam/www/public/us/en/documents/specification-updates/i218-i219-ethernet-connection-spec-update.pdf?asset=9561
* Minor bug fixes.
* Cosmetic changes.

**Changelog for 3.3.5.10**
* Build fixes for newer kernels.
* Added support to Red Hat 7.4.
* Added support to Suse Enterprise Linux 12- SP3.
* Minor bug fixes.

**Changelog for 3.3.5.3**
* Build fixes for newer kernels.
* Added support to Red Hat 7.3

**Changelog for 3.3.5**
* Build fixes for newer kernels.
* 82579: Disable FLR capability to prevent the 82579 from hanging.

Once an 82579 device is attached to a VM, stop the VM and return the ownership to the host, then try to assign to another VM. This can cause an adapter hang.

**Changelog for 3.3.4**
* Build fixes for newer kernels.

**Changelog for 3.3.3**
* Initial support for the following devices:
  * Ethernet Connection (4) I219-LM
  * Ethernet Connection (4) I219-V
  * Ethernet Connection (5) I219-LM
  * Ethernet Connection (5) I219-V
* Change installation folder to: /lib/module/<KERNEL_VERSION>/updates/drivers/net/ethernet/intel/e1000e/ Instead of: /lib/module/<KERNEL_VERSION>/kernel/drivers/net/ethernet/intel/e1000e/
* Fix msi-x interrupt automask

**Changelog for 3.3.1**
* Initial support for Ethernet Connection (3) I219-LM
* Fix builds on newer kernels
* Fix timing issues between the ME firmware and the LAN controller

**Changelog for 3.2.7.1**
* Fix build on SLES12, RHEL 7.2
* Fix builds on newer kernels

**Changelog for 3.1.0**
* Fix - Compile on Ubuntu 14.04
* Fix - Makefile for newer kernels with 1588 clock unconfigured

**Changelog for 3.1.0**
* Fix - ethtool register tests updated for new code
* Cleanup - update function calls to new non-deprecated versions
* Fix - Runtime PM interfering with EEE in Sx states
* Fix - EEE in S5 use same workaround as S3 and S4
* Fix - Compile tag wrapping for Runtime PM
* Fix - loading driver when cable out not initializing hardware correctly
* Fix - ethtool interacting with Runtime PM
* Fix - initialization of skbuff's
* Cleanup - return values should use true/false instead of 1/0
* Add - timeout for HW time stammping
* Add - ethtool statistic for HW timestamp timeouts
* Cleanup - driver warning messages
* Cleanup - various code style cleanups
* Fix - include VLAN_HEADER in MTU calculation when changing MTU
* Fix - make more intelligent choices when grabbing address registers on ME platforms
* Add - send notification and handle case when programming address register fails
* NOTE - due to a backporting issue, there is a problem compiling this driver
	under Ubuntu 14.04.

**Changelog for 3.0.4.1**
* Workaround - packet loss when exiting K1 on 82579 parts
* Fix - 32 bit DMA mask handling

**Changelog for 3.0.4**
* Compat - Compile issue on RHEL 6.5
* Fix - Verify PTP register reads on 82574/82583 parts
* Fix - Add lock to PTP register writes to prevent concurrent access
* Cosmetic - Cleanup GPL header info and change copyright dates
* Cleanup - remove unneeded pointer references
* Fix - Energy Efficient Ethernet in Sx states
* Fix - Device state changes while in Ultra Low Power PHY mode
* Fix - Device state changes while MAC is in D3 power state
* Compat - Account for the deprecation of random_ether_addr
* Compat - Adjusted data structures for compatibility with Linux tip-of-tree
* Cleanup - remove kernel namespace pollution
* Fix - ULP functionality
* Add - Dynamic Latency Tolerance Reporting to allow deeper C-States on supported parts
* Compat - Compatibility issues with 2.4.x kernels
* Cleanup - Changes put in place to account for new checkpatch.pl
* Fix - Feature controlled compile tags for specific kernel configurations
* Fix - Runtime PM interfereing with EEE in Sx
* Change - Don't automatically disable EEE advertising when disabling EEE, Leave the control to the user running ethtool --set-eee
* Cleanup - remove obsolete member of adapter stuct
* Fix - Handle Management Engine blocking PHY access for a time after resests
* Fix - Semaphore imbalance for 82573 parts
