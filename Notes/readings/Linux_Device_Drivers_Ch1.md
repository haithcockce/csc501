Chapter 1: An Introduction to Device Drivers - O’Reilly
=======================================================

### The Role of the Device Driver
* The role is to provide mechanisms and not a policy, so it needs to be flexible
* Main issues of programming problems can be divided into what capabilities are provided (mechanisms, such as sftp servers) and how they can be used (policy, such as the clients that interact with those sftp servers)
  * A floppy drive driver should present the drive as a continuous array of blocks while the OS determines how to interact with the device (IE a fs or swap)
* Drivers should deal only with making the hardware available. Sometimes policy decisions must be made
* Make sure to emphasise the following: 
  * synchronous and asynchronous access 
  * allow the device to be opened multiple times
  * exploit all the capabilities of the hardware
  * lack of required software to provide policy-related decisions (IE allow any software to create policy decisions?)

### Splitting the Kernel
* **Process Management** Scheduling; interprocess communications; creation, maintenance, and destruction of processes; input to and output from processes
* **Memory Management** Allows processes to interact with the VMA space that translates to physical memory 
* **Filesystems** representing interactions with and organizations within devices 
* **Device Control** these kinds of operations that allow interactions with devices. The code that performs such is a device driver. 
* **Networking** packets must be collected, identified, and dispatched to processes/network interfaces; routing and address resolution is handled within the kernel; and control of program execution based on net activity is also handled within the kernel 

### Loadable Module
* _Modules_ are pieces of code that can be added to the kernel at run time extending its functionality. Modules can be divided into fuzzy classes based on their functionality
* Classes
  * **Character Module**
    * Char devices, devices accessed as a stream of bytes like consoles and serial ports. 
    * Devices are accessed like a filesystem (fs) node in /dev
    * Typically these are accessed sequentially but can move back and forth in them 
  * **Block Module**
    * Like Char modules, they are used to access streams of data, but block devices themselves can handle only 512 bytes at a time (this is a block and can actually be a larger power of 2). 
    * Virtually, no difference exists from userspace, and the difference lies in how the data is managed within the kernel/driver
    * The interface to the devices however will vary greatly from char devices. 
  * **Network Module**
    * _Interfaces_ are devices (either hardware or software) that allow data exchange between hosts and is in charge only of sending and receiving packets
    * The devices and their drivers do not need to know which packets match to which transactions
    * Though interactions with these devices are typically stream oriented, the devices are concerned only with sending and receiving packets and therefore do not have a fs node. 
* Additional classes can be considered such as USB classes (since you can have a USB networking, storage, or serial device)
* Modules can be completely software related as well. Filesystems are modules since they translate high level interactions with files to interactions with the low level block devices and are independent of the kinds of interactions being performed

### Security Issues
* Kernel code enforces security checks for who is loading the module. 
* Security policies however are handled via the SA
* The driver should check if the user is privileged before using the driver especially since the driver can fuck with global resources or other users
* The code must also not be open for exploitation (IE buffer overflow)
