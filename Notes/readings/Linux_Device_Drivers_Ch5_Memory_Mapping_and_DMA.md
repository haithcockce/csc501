# Memory Management in Linux

### Address Types
Virtual memory allows a layer of abstraction between processes and the system's physical memory, allowing a process to think it has more memory than it actually has or that the system even has installed and even allowing addressing directly to device memory. 
* __User virtual addresses__ These are the addresses seen by the userspace process and used for addressing within userspace
* __Physical Addresses__ the actual addresses of the pages of memory and used between the processor and main memory
* __Bus Addresses__ Addresses used between peripheral buses and memory. Devices can have an IOMMU which can offer similar abstractions to devices that virtual memory offers userspace processes (IE scattering device memory even though the memory appears contiguous to the device).
* __Kernel Logical Addresses__ Normal addresses within the kernel and treated like physical memory. This is essentially contiguous addresses and the kernel does a 1:1 mapping of the memory with this. 
* __Kernel Virtual Addresses__ Similar to virtual memory for userspace, we perform "random" mappings to smooth out memory allocations. 

##### Physical Addresses and Pages
Memory is divided into pages (4 KiB in Linux by default) and the majority of memory operations are done on a per-page basis. To access a specific part of a page, the address requested is broken into parts, where the higher order (left most) part indicates the page number (or PFN, the _page frame number_ ) while the lower order (right part) indicates the offset into the page. 

##### High and Low Memory
Kernel stuff sits in low mem while user stuff sits in high mem. In order to access any memory, the page must be mapped which is held within the kernel virtual addresses. In general, low memory is memory that has logical kernel addresses while high memory does not have logical kernel addresses mapping to them. 

### The Memory Map and Struct Page
Pages of memory are represented as a `struct page` and contain a ref count, the kernel virtual address of the page (if mapped, NULL otherwise), and a bitmap of flags describing its status. These pages are kept track of in lists of `mem_map`s, one per NUMA node (where only 1 of these means UMA). Helper functions: 
* __`virt_to_page()`__ translates a kernel logical address to a `page` pointer. 
* __`pfn_to_page()`__ translates a page frame number to a `page` pointer. 
* __`page_address()`__ translates a `page` pointer to its kernel virtual address
* __`kmap()`, `kunmap()`__ translates a `page` pointer to a kernel virtual address, creating a new mapping if need be. These are freed with `kunmap`. 
* __`kmap_atomic()`, `kunmap_atomic()`__ high performance mapping and freeing using slotts to indicate where the request originatied (either from userspace or from an interrupt). 

##### Page Tables
A multileveled tree that allows mapping from virtual pages to physical pages. 

##### Virtual Memory Areas
Kernel data structure used to manage distinct regions of a process' address space. It is a homogeneous region in the VM of a process and is backed by the same object (like a file in tmpfs or otherwise or swapspace). VMAs consist of:
* _text_ or the program's executable code
* Areas for data
    * Initialized data
    * _BSS_ or uninitialized data (Block Started by Symbol)
    * Program stack
* An area for each active memory mapping
The VMAs for a process are seen in `/proc/<PID>/maps` and the columns represent `start-end, permissions, offset, maj:min, inode, image`. The start-end are the addresses for the memory area, offset is for the offset into the file the VMA maps to, maj:min is fort the device containing the file, and image is the name of the file (such as the executable).  

All VMA is represented by a `vm_area_struct`:
* __`vm_start`, `vm_end`__ The virtual address range of this VMA
* __`vm_file`__ The `file` structure backing this VMA if any
* __`vm_pgoff`__ page offset into the file. 
* __`vm_flags`__ bitmap of descriptor flags
* __`vm_operations_struct`__ A bundle of operations you can invoke on the VMA
* __`vm_private_data`__ private data for the VMA
You can mmap device memory into a process' VMA directly. However, device drivers must actually provide some of the information on this matter including pointers to `open, close,` etc. The following must be implemented for processes to be able to map to a device's memory:
* __`open()`__ Creates a new reference to the VMA unless it has not created yet. In this case, the device driver mmap is called
* __`close()`__Destroys a VMA. The VMA is opened and closed only once by each process using it. 
* __`nopage()`__ called when a page is accessed for a VMA that is not yet in memory
* __`populate()`__ prefaults pages into memory though this is not necessary for drivers. 

##### The Process Memory Map
A `mm_struct` object's purpose is to hold information about the VMA, page tables, and other housekeeping information about a Process's mapped memory along with locks for accessing such. 

### Memory Mapping Device Operations
Memory mapping can allow direct access to deivce memory. You can check out the addresses of what is mapped in `/proc/iomem`. If a process has mapped something, you can see it in `/proc/<PID>/map` and from there you can compare the address within `/proc/<PID>/map` to `/proc/iomem` to see what is correlates to if it is something like device memory. When reading and writing to the VMA range that is mapped to device memory, you actually are accessing the device. However, as noted, memmgmt is done at a page level of granularity, so pages might be larger than that address space. The mapping is arch specific, however, but when applicable, mapping directly to devices is a performance boost (such as writing directly to video ram, or mapping control registers to memory for some peripherals). To implement the ability to mmap something directly to device memory, simply implement mmap, which means building page tables and possibly implement your own `vma->vm_ops`. 

##### `mmap` Implementation Using `remap_pfn_range`
This can be thought of as the all at once method via `remap_pfn_range()` (used for situations where pfn point to system memory) and `io_remap_pfn_range()` (where the pfn points to IO memory). If doing a linear mapping of memory, `remap_pfn_range()` can do it very easily. When creating the VMA for a process either by `exec()`, `fork()`, etc, the kernel does all the work of duplicating the pages. As such, while you can add functionality to the driver on these operations via the implementing of `open()` and `close()` which are called during these VMA creation, you do not need to redo any work. Once implemented, simple add them in the `vm_operations_struct`. These should be assigned within the implementation of the mmap. The action of using `remap_pfn_range` will go ahead and map the entirety of the range provided. 
* The downside to this is that it is not flexible. If you need memory, you only get all of it. 
* Another is that you can not remap this, which means you can not shrink or grow the VMA (which is just more detailed version of the above point) 
* The benefit of this however is that all of memory needed is mapped, so no page faults will occur
* Another benefit to this is that PCI memory can be mapped like this since PCI memory will be addressed above the system memory and this memory will not have any `page` objects representing them. 

##### `mmap` Implementation Using `nopage`
Allows VMA to shrink and grow because the memory is not allocated all at once. When shrinking a VMA, the unwanted pages (the pages trimmed from the VMA that are deemed not very useful) are simply flushed to the device and the driver does not need to necessarily know about it. When growing, `nopage` is called to fault in pages from the device. For this, you must properly manage the ref count increments via `get_page()`. In the mmap call, like in the remap_pfn_range implementation, you must set the `vm_ops` to know how to interact with the VMA you are mapping. However, after implementing the mmap, you then also have to implement nopage. This implementation must perform the necessary calculations to translate the desired address to the physical page itself, check if the PFN is valid, and grab the page via the previously described `pfn_to_page`, the increment the ref count with `get_page()` then return the page (pointer). REMINDER: The physical address is calulated by breaking apart the desired virtual address and shifting the address around to get the various location clues. The previously defined functions help actually perform the translation. However, if this `nopage()` methi=od is left unimplemented, and therefore NULL, then we simply fault in zero pages so we can use it for some thing (like BSS) rather than simply segfaulting. 
* The downside to this is that you are guarenteed a minor fault if the page requested is not in memory 
* Furthermore, PCI memory can not be mapped like this as their physical addresses are above main memory altogether (therefore no `page` exists for their addresses), so you can not fault pages in. 
* The benefit of this is that the mapped memory can grow and shrink if need be or even fully relocate (why relocate, Idk).  

##### Remapping IO Regions
Most of the time, we don't want to map all of a device's memory, but only a subset of it. For this, the application may try and grab more memory than what is available to provide from the range in the device, so you have to make sure additional extensions to the VMA won't be created, as they will not be backed by anything. You could simply implement a `nopage()` that will error out everytime. 

##### Remapping RAM
`remap_pfn_range()` provides access only to reserved memory and physical addresses above physical memory, so remapped pages only bring in zero pages, but this is still good for mapping PCI devices. For mapping userspace pages to physical pages, you would instead use `nopage()` to fault them in one at a time. 
