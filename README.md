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

![Paging Process](https://user-images.githubusercontent.com/32425672/36358889-ee1b5380-14c9-11e8-9f5d-4df9de38a156.png)
![Program Process](https://user-images.githubusercontent.com/32425672/36358891-f2f32bbc-14c9-11e8-80b0-f42b602007c2.png)


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

## Page Replacement Algorithm 
Since all logical addresses are less than 256, there are sufficient places for the page table to loaded all pages into the physical memory. Therefore the replacement algorithms are demonstrated in the TLB.
3 algorithms are implemented: 
- FIFO
- LRU 8 Reference Bits
- LRU Clock Replacement.

## To Run
    g++ self.cpp
    ./a.out [Input file] [Flag]
    Example: ./a.out addresses.txt -l
#### or
    gcc main.c
    ./a.out addresses.txt

## Example Output: 
    ./a.out addresses.txt -l

    ...
    WELCOME TO VIRTUAL MEMORY MANAGER
    Result is outputed to the result.txt file
    Algorithm: FIFO [-f], LRU [-l], CLOCK [-c]

    Page Number: 47
    Frame Number: 10
    Offset: 75
    Virtual Address: 12107 Physical Address: 2635 Value: -46

    Page Replacement Algorithm: LRU
    Number of translated address: 1000
    Page Fault: 244
    Page Fault Rate: 0.244
    TLB Hits: 60
    TLB Hits Rate: 0.06
    TLB index: 16
    ...