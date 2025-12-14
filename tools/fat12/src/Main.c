#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define PrintNewLine() write(1, "\n", 1)
typedef struct BPB{
    uint8_t m_Start[3];
    uint8_t m_OEMIdentifier[8];
    uint16_t m_BytesPerSector;
    uint8_t m_SectorsPerCluster;
    uint16_t m_ReservedSectors;
    uint8_t m_FatCount;
    uint16_t m_RootEntries;
    uint16_t m_TotalSectors;
    uint8_t m_MediaDescriptorType;
    uint16_t m_SectorsPerFAT;
    uint16_t m_SectorsPerTrack;
    uint16_t m_Heads;
    uint32_t m_HiddenSectors;
    uint32_t m_LargeSectorCount;
    // EBPB
    uint8_t m_DriveNumber;
    uint8_t m_WindowsNTFlags;
    uint32_t m_VolumeId;
    uint8_t m_VolumeLabelString[11];
} __attribute__((packed)) BPB;

typedef struct DirectoryEntry{
    uint8_t m_FileName[11];
    uint8_t m_Attributes;
    uint8_t m_Reserved;
    uint8_t m_CreationTime;
    uint16_t m_CreationDate;
    uint16_t m_DateCreated;
    uint16_t m_LastAccessed;
    uint16_t m_Unused;
    uint16_t m_LastModificationTime;
    uint16_t m_LastModificationDate;
    uint16_t m_FirstLogicalCluster;
    uint32_t m_Size;
}__attribute((packed)) DirectoryEntry;

typedef struct CHS{
    uint32_t m_Cylinder;
    uint32_t m_Head;
    uint32_t m_Sector;
} CHS;
char* ReadFile(char* disk, BPB* bpb, DirectoryEntry* entry, char fileName[11]){
    if(entry->m_Size < 1)return NULL;
    bool t = false;
    char* data = (char*)malloc(entry->m_Size);
    size_t currentFilled = 0;
    uint8_t* fatTable = data + bpb->m_BytesPerSector;
    //uint8_t activeCluster = entry
    //mempcpy(data, disk + (bpb->m_FatCount + bpb->m_ReservedSectors) * bpb->m_BytesPerSector, entry->m_Size);
    uint16_t logicalSector = entry->m_FirstLogicalCluster;
    size_t clusterLocation = (bpb->m_SectorsPerFAT * bpb->m_FatCount + bpb->m_ReservedSectors) * bpb->m_BytesPerSector + bpb->m_RootEntries * sizeof(DirectoryEntry); 
    printf("ClusterLocation %u\n", clusterLocation);
    printf("First cluster %u\n", entry->m_FirstLogicalCluster);
    mempcpy(data, disk +clusterLocation + bpb->m_SectorsPerCluster * entry->m_FirstLogicalCluster, 15);
    return data;
}
int CHSToLBS(BPB* bpb, int c, int h, int s){
    return 0;
}
CHS LBAToCHS(BPB* bpb, int lba){
    CHS chs;
    unsigned int s = bpb->m_SectorsPerTrack;
    unsigned int temp = lba / s;
    chs.m_Sector = (lba % s) + 1;
    chs.m_Head = temp % s;
    chs.m_Cylinder = temp / bpb->m_Heads;
    return chs;
}
int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "bad usage - fat12 filesystem.bin");
        return -1;
    }
    int file = open(argv[1], O_RDONLY);
    if(!file){
        fprintf(stderr, "Failed to open file %s\n", argv[1]);
        return -2;
    }
    printf("Opened fs %s\n", argv[1]);
    off_t size = lseek(file, 0, SEEK_END);
    if(size == -1){
        fprintf(stderr, "Failed to go to end of file\n");
        return -3;
    }
    lseek(file, 0, SEEK_SET);
    char* data = malloc(size);
    struct BPB bpb;
    if(size < 2560){
        fprintf(stderr, "Filesystem is too small, only %u bytes\n", size);
        return -4;
    }
    printf("File size %u\n", size);
    ssize_t bytesRead = read(file, data, size);
    memcpy(&bpb, data, sizeof(struct BPB));

    write(1, bpb.m_OEMIdentifier, 8);
    PrintNewLine();

    write(1, bpb.m_VolumeLabelString, 11);
    PrintNewLine();

    printf("Sectors per cluster %u\n", bpb.m_SectorsPerCluster); 
    printf("Fat count %u\n", bpb.m_FatCount); 
    printf("reserved count %u\n", bpb.m_ReservedSectors);
    printf("Sectors per fat %u\n", bpb.m_SectorsPerFAT); 
    printf("Total Sectors %u\n", bpb.m_TotalSectors);
    printf("Sectors track %u\n", bpb.m_SectorsPerTrack);
    printf("Hiden Sectors %u\n", bpb.m_HiddenSectors);
    printf("Media Descriptor Type %X\n", bpb.m_MediaDescriptorType);
    printf("Heads Per Size %u\n", bpb.m_Heads);
    printf("Root Entries %u\n", bpb.m_RootEntries);
    DirectoryEntry* root = (DirectoryEntry*)(data+ (512 * (1 + bpb.m_FatCount * bpb.m_SectorsPerFAT)));

    // Reading Main File
    for(size_t i = 0; i < bpb.m_RootEntries; i++){
        DirectoryEntry* entry = &root[i];
        if(strncmp(entry->m_FileName, "STAGE6  BIN", 11)){
            continue;
        }
        printf("Got file");
        printf("Data size %d\n", entry->m_Size);
        char* fileData = (char*)malloc(entry->m_Size);
        size_t cluster = entry->m_FirstLogicalCluster;
        size_t offset = bpb.m_BytesPerSector * (bpb.m_ReservedSectors + bpb.m_SectorsPerFAT * bpb.m_FatCount) + sizeof(DirectoryEntry) * bpb.m_RootEntries + (cluster - 2) * bpb.m_SectorsPerCluster * bpb.m_BytesPerSector;
        size_t lba = (bpb.m_ReservedSectors + bpb.m_SectorsPerFAT * bpb.m_FatCount) + (((bpb.m_RootEntries * sizeof(DirectoryEntry) + (bpb.m_BytesPerSector - 1)) / bpb.m_BytesPerSector)) + (cluster - 2) * bpb.m_SectorsPerCluster;
        printf("Logical cluster %u\n", bpb.m_SectorsPerCluster);
        printf("offset %u\n", offset);
        printf("LBA id is %u\n", lba);
        CHS chs = LBAToCHS(&bpb, lba);
        printf("CHS %u %u %u\n", chs.m_Cylinder, chs.m_Head, chs.m_Sector);
        memcpy(fileData,
            data + offset
        ,entry->m_Size);
        for(size_t j = 0; j < entry->m_Size; j++)printf("%c", fileData[j]);
        //printf("Data %.*s \n", entry->m_Size, fileData);
        free(fileData);
    }
    //char* fileData = ReadFile(data, &bpb, root, "SOURCE .TXT");
    //if(fileData)printf("File data %s\n", fileData);
    //else fprintf(stderr, "Failed to read file\n");
    //free(fileData);
    free(data);
    printf("Closing File");
    close(file);
}
