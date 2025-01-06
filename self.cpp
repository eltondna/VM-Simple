#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
using namespace std;

// MACRO
#define FRAMES        256
#define FRAME_SIZE    256
#define PAGE_SIZE     256
#define TLB_SIZE      16
#define ADDRESS_SIZE  10
#define CHUNK         256
#define PAGE_MASKER   0xFFFF
#define OFFSET_MASKER 0xFF

// Data Structure 
struct page_entry
{
    int page_number;
    int frame_number;
};
int         RAM[FRAME_SIZE][FRAMES];
page_entry  PageTable[PAGE_SIZE];
page_entry  TLB[TLB_SIZE];


// Record current PageNumber and FrameNumber
int firstAvailablePageIndex = 0;
int firstAvailableFrameIndex = 0;
int TLB_index = 0;

// Functions
void get_page(int logical_address);
int  read_from_store(int pageNumber);


// 3 Swapping Algorithms
void FIFO(int pageNumber, int frameNumber);

void LRU8Bit(int pageNumber, int frameNumber);
uint8_t referenceBytes[TLB_SIZE];

void SecondChance(int pageNumber, int frameNumber);
bool referenceBits[TLB_SIZE];

string mode;

// File Descriptor 
int address_fd;
int backing_store_fd;

// Statistic Data
int pageFault = 0;
int TLBHit = 0;


void get_page(int logical_address){
    int pageNumber  = (logical_address & PAGE_MASKER)  >> 8;
    int offset      = logical_address & OFFSET_MASKER;
    int frameNumber = -1;
    int value = -1;

    int i;
    for (i = 0; i < TLB_index; i++){
        if (TLB[i].page_number == pageNumber){
            frameNumber = TLB[i].frame_number;
            TLBHit++;
            // cout << TLBHit << ' ' << pageNumber << ' ' << frameNumber << logical_address << endl;
            break;
        }
    }
    // TLB Miss
    if (frameNumber == -1){
        for (i = 0; i < firstAvailablePageIndex; i++){
            if (PageTable[i].page_number == pageNumber){
                frameNumber = PageTable[i].frame_number;
                break;
            }
        }
        // Page Fault
        if (frameNumber == -1){
            frameNumber = read_from_store(pageNumber);
            pageFault++;
        }
    }
    if (mode.compare("LRU") == 0) 
        LRU8Bit(pageNumber, frameNumber);
    else if (mode.compare("CLOCK") == 0) 
        SecondChance(pageNumber,frameNumber);
    else FIFO(pageNumber, frameNumber);

    value = RAM[frameNumber][offset];

    cout << "Page Number: "  << pageNumber  << '\n'
         << "Frame Number: " << frameNumber << '\n'
         << "Offset: "       << offset      << '\n';

    cout << "Virtual Address: "   << logical_address 
         << " Physical Address: " << ((frameNumber << 8) | offset)
         << " Value: "            << value << '\n' << endl;
}


int read_from_store(int pageNumber){
    signed char buffer[CHUNK];

    if (lseek(backing_store_fd, pageNumber * CHUNK, SEEK_SET) == -1)
        fprintf(stderr, "Error on locating byte location\n");
    
    if (read(backing_store_fd, buffer, CHUNK) == -1)
        fprintf(stderr, "Error on reading the binary file\n");
    
    int i;
    for (i = 0; i < CHUNK; i++){
        RAM[firstAvailableFrameIndex][i] = buffer[i];
    }

    PageTable[firstAvailablePageIndex].page_number = pageNumber;
    PageTable[firstAvailablePageIndex].frame_number = firstAvailableFrameIndex;

    firstAvailableFrameIndex++;
    firstAvailablePageIndex++;

    return PageTable[firstAvailablePageIndex-1].frame_number;

}

void FIFO(int pageNumber, int frameNumber){
    // FIFO Algorithm
    int i;
    for (i = 0; i < TLB_index; i++){
        if (TLB[i].page_number == pageNumber){
            for (; i < TLB_index; i++){
                TLB[i] = TLB[i+1];
            }
            TLB[TLB_index-1].page_number = pageNumber;
            TLB[TLB_index-1].frame_number = frameNumber;
            return;
        }
    }
    
    if (i == TLB_index){
        if (i == TLB_SIZE){
            for (int j = 1; j < TLB_index; j++)
                TLB[j-1] = TLB[j];
            // cout << pageNumber << ' ' << TLB_index<< endl;
            TLB[TLB_index-1].page_number = pageNumber;
            TLB[TLB_index-1].frame_number = frameNumber;
        }else{
            TLB[TLB_index].page_number = pageNumber;
            TLB[TLB_index].frame_number = frameNumber;
            TLB_index++;
        }
    }

    // for (int j = 0; j < TLB_index; j++){
    //     cout << TLB[j].page_number << ' ' << TLB[j].frame_number << endl;
    // }
    // cout << "\n\n\n";
}
void LRU8Bit(int pageNumber, int frameNumber){
    int i;
    bool flag = false;

    for (i = 0; i < TLB_index; i++){
        if (TLB[i].page_number == pageNumber){
            referenceBytes[i] = ((referenceBytes[i] >> 1) |(1 << 7));
            TLB[i].frame_number = frameNumber;
            flag = true;
            break;
        }
    }

    uint8_t lowest = 255;
    int position = i;

    // TLB Miss 
    if (!flag){
        // TLB Full 
        if (i == TLB_SIZE ){
            for (int j = 0; j < TLB_SIZE; j++){
                if (referenceBytes[j] == 0){
                    position = j;
                    break;
                }
                if (lowest > referenceBytes[j]) {
                    lowest = referenceBytes[j];
                    position = j;
                }
            }
        }else TLB_index++;
        // Swap Page 
        TLB[position].page_number = pageNumber;
        TLB[position].frame_number = frameNumber;
        // Reset reference Bit
        referenceBytes[position] = 0;
    }

    // Update History bit
    for (int j = 0; j < TLB_index; j++){
        if (j == position) continue;
        referenceBytes[j] = (referenceBytes[j] >> 1);
    }
}
void SecondChance(int pageNumber, int frameNumber){
    int i;
    for (i = 0; i < TLB_index; i++){
        if (TLB[i].page_number == pageNumber){
            referenceBits[i] = 1;
            TLB[i].frame_number = frameNumber;
            return;
        }
    }
    if (i == TLB_SIZE){
        for (int j = 0; j < TLB_index; j++){
            if (referenceBits[j] == 0){
                TLB[j].page_number = pageNumber;
                TLB[j].frame_number = frameNumber;
                referenceBits[j] = 1;
                return;
            }
            referenceBits[j] = 0;
        }
    }else{
        TLB[TLB_index].page_number = pageNumber;
        TLB[TLB_index].frame_number = frameNumber;
        referenceBits[TLB_index] = 1;
        TLB_index++;
    }
}


int main(int argc, char const *argv[])
{
    cout << "WELCOME TO VIRTUAL MEMORY MANAGER" << endl;
    cout << "Result is outputed to the result.txt file" << endl;
    cout << "Algorithm: FIFO [-f], LRU [-l], CLOCK [-c]" << endl;

    if (argc == 3){
        mode = (strcasecmp(argv[2], "-f") == 0) ? "FIFO" :
               (strcasecmp(argv[2], "-l") == 0) ? "LRU"  :
               (strcasecmp(argv[2], "-c") == 0) ? "CLOCK": "";
        if (mode == "") 
            cerr << "Invalid Flag" << endl;

    }else if (argc < 2)
        cerr << "Usage: ./a.out [input file] [flag: optional]" << endl;
    


    address_fd       = open(argv[1],O_RDONLY);
    backing_store_fd = open("BACKING_STORE.bin",O_RDONLY);

    if (address_fd == -1)
        cerr << "Error opening addresses.txt " << argv[1] << endl;
        
    if (backing_store_fd == -1)
        cerr << "Error opening BACKING_STORE.bin " << endl;

    int translated_address = 0;
    int logical_address = 0;
    FILE * stream = fdopen(address_fd, "r");
    
    char address[ADDRESS_SIZE];

    while (fgets(address, ADDRESS_SIZE, stream))
    {
        logical_address = atoi(address);
        get_page(logical_address);
        translated_address++;
    }
    cout << "Page Replacement Algorithm: " << mode << endl;
    cout << "Number of translated address: " << translated_address << endl;

    double pfRate = pageFault / (double) translated_address;
    double TLBRate = TLBHit / (double) translated_address;

    cout << "Page Fault: " << pageFault << endl;
    cout << "Page Fault Rate: " << pfRate << endl;
    cout << "TLB Hits: " << TLBHit << endl;
    cout << "TLB Hits Rate: " << TLBRate << endl;

    fclose(stream);
    close(backing_store_fd);
}