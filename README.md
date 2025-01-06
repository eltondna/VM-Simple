# Virtual Memory Maneger 

A Virtual Memory Manager translates logical address to physical address.

The program read from a file containing logical addresses, using TLB , Page Table to tanslate each logical address to its physical address and output the value of the byte stored at the translated physical address in the binary file.

## Background & Problem Description: 
- The virtual memory manager simulates the process of logical to physical address translation
- Logical Address  : Page Number  +  Offset
- Physical Address : Frame Number +  Offset 

## Step: 
- Extract "Page Number" and "Offset" (Logical Address)
- Check if the page is cached 
- Consult TLB: 
    - If in TLB         --> TLB Hits --> Get the Frame Number
    - If not in TLB     --> Consult the Page Table

- Consult the Page Table: 
    - If in Page Table  --> Get the Frame Number
    - If not            --> Page Fault (Page not in physical memory)

    - Page Fault: Read the data from the binary to the Physical Memory



To handle the page fault, BACKING_STORE.bin is consulted. It represents a hard drive that can be accessed randomly. Data are aligned according to page number. Therefore using page number as index, a whole frame could be read and allocated to the physical memory. After that, page table and TLB is updated and data are acquired using offset.


## Configuration: 
    - Page Table            : 256 entries
    - TLB                   : 16  entries
    - Frame                 : 256  
    - Frame Size            : 256 bytes
    - Data                  : Signed Integer
    - Physical Memory       : 2D Array (Dimension: Frame x Frame Size)
    - TLB & Page Table      : Array 
    - BackingStore          : Binary File (BACKING_STORE.bin)
    - Input && Output File  : addresses.txt && output.txt

## Output: 
    - Logical Address
    - Physical Address
    - Offset
    - Page Number 
    - Frame Number
    - Page Fault
    - Page Fault Rate 
    - TLB Hit 
    - TLB Hit Rate