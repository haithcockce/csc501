The UNIX Time-Sharing System 
============================

### Introduction
* UNIX was cheap in man power and money and offered more features than most systems offered by hardware vendors including software that offers recreation
System is self-supporting
* UNIX software is maintained in UNIX, programs are written locally, files can be edited in UNIX, etc

###Hardware and Software Environment
* PDP-11: 16-bit words, 144 KiB of memory, 42 KiB is used for UNIX alone, but a system could have as little as 50 KiB of memory
* Several disks, some fixed, others swappable as cartidges, paper-tape reader punch, 9track, magnetic tapes, DECtape
* 14 communication interfaces, and a dataset interface for line printer
Also supported unique interfaces, such as a tube display, voice recognizer, PicturePhone, phototypesetter, etc
* After rewriting UNIX in C, it is ⅓ larger, but it is easier to understand and modify as well as included functional improvements

### The Filesystem

##### Ordinary Files
* Files contain whatever data is placed into them and is seen only as such by the OS
* While user programs set and manipulate the files and may expect particular formats or structures, this is controlled from the programs and not the OS

##### Directories
* Provides mappings between file names and their corresponding files
* These files work like ordinary files but can not be written to by unprivileged users
* System has directories for its uses and starts with root (/). System also uses a directory for all its binaries, but those can reside anywhere
* File names are 14 characters long
* Linking when the name for the same file appears in multiple locations in directories. Entries in a directory point to file metadata that points to the file itself, so files don’t actually point to the data in question, though a file is “deleted” when all links for it are removed. 

##### Special Files
* Device files under /dev that allow interactions with a specific device. Reads and writes to these files indicate read and write operations sent to the device driver and the device itself. 
* Useful to represent devices like files since both files and devices are read from and written to, file names can be symantically the same as the device name, and file protections then can be applied to devices to protect them from use. 

##### Removable File Systems
* While the root filesystem needs to remain on the same device, not all of the filesystem needs to reside on the same device, 
* Mounting causes a leaf node in the fs hierarchy to refer to the root of the other fs
* Internal nodes of separate fs may not refer to each other since the nodes will then have multiple roots
* The parent reference for a root directory refers to itself

##### Protection
* 7 bits are used to determine who has RWX access. The 7th bit modifies who is the executor of the file, IE when set, if the UID of the file owner is 100 and the UID of the user executing the file is 200, the file is executed as if UID 100 is executing it (set-user-id or SUID bit). This occurs only when executing. 
  * useful if only a specific program needs to modify a file and no one or nothing else. 
  * SUID does not require root to set it on personal files

##### I/O Calls
* No distinction between random or sequential IO and we do not care about the differences between devices (we shouldn’t), and the size of a file is determined by the location of the highest bit set. Predetermined file size is not possible and unnecessary 
* To R/W to a file, you need a filep which is a file descriptor (via `open()`).
* New files are created with `create()` which either creates the file or truncates the existing file to 0 length. 
* User-visible locks do not exist on the system because they are unecessary. The kernel maintains locks to maintain concurrent access. 
* R/W are sequential. So a read of n bytes sequetially will place the reader at the n+1 byte. To that end, the R/W of n bytes causes the probe to advance n bytes. 
* `n = read(filep *, buffer, cnt)`, `n = write(filep *, buffer, cnt)`
  * `cnt` bytes are transfered from/to `buffer` to/from the location designated by `filep *` returning `n` bytes bytes that were actually written/read
  * Reaching the end of the file means the filep matches the length of the file
* Direct access can be achieved by starting from the start of the file and *seeking* to the location in the file. 
  * `location = seek(filep *, base, offset)`
* Other file operations exist

### Implementation of the FS
* Directories contain the file name of a file and it's pointer (*index number* or *i-number*) which points to an entry (*inode*) in a table (*i-list*) which contains the following: 
  * File owner
  * protection bits
  * physical address of the contents 
  * size
  * last touch timestamp
  * count of links referring to this file
  * bits indicating if it is a directory, special file, and if it is "small"/"large"
* Creation or opening of a file translates the file name to the inode.
  * Creating a file allocates an entry in the i-list while creating a link simply creates the directory entry with the name and i-number and the inode's link count is incremented. Deleting is removing the file name and i-number from the i-list and decrementing the inode. 

##### Block device and translation
* inodes contain addresses for the device addresses. *Small* files fit into those addresses or less while large files may have the addresses point to an indirect block of 256 addresses (meaning a max of 1048576 B, though this is not the same nowadays). 
  * For special files, the first of the 8 addresses indicate the pair of bytes (device type and number or Major and Minor numbers) that create the device name where the major number indicates the routines for dealing with the device
 

