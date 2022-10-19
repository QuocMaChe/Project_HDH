#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <windows.h>
#include <fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include <string.h>
#include<vector>
#include<string>
#include <atlstr.h>
#include <fcntl.h>
#include"Util.h"

using namespace std;

//Cấu trúc BootSector
struct BOOTSECTORFAT32
{
    BYTE JUMP[3];
    BYTE OEM[8];
    WORD BytePerSector;                     // 	Số byte của một Sector      (B - 2)
    BYTE SectorPerCluster;                  // 	Số Sector của một Cluster   (D - 1) SC
    WORD ReservedSector;                    // 	Số Sector của BootSector    (E - 2) SB
    BYTE FatNum;                            // 	Số bảng FAT                 (10 - 1) NF
    WORD EntryRDET;                         // 	Số Entry của RDET           (11 - 2) SRDET
    WORD LowNumberSectors;                  // 19-20	Total number of sector in the filesystem
    BYTE DeviceType;
    WORD SectorPerFat16;
    WORD SectorPerTrack;                    // Số Sector của một Track      (18 - 2)
    WORD HeadPerDisk;

    DWORD NumberHiddenSectors;
    DWORD HighNumberSectors;
    DWORD SectorPerFat32;                   //  Số Sector của một bảng FAT  (24 - 4) Sf
    WORD Bit8Flag;
    WORD FAT32Ver;
    DWORD FirstRDETCluster;                 //  Cluster bắt đầu của RDET    (2C - 4)
    WORD AddiInfoSector;
    WORD BackupSector;
    BYTE LaterVerReserved[12];

    BYTE PhysicDisk;
    BYTE Reserved;
    BYTE Signature;
    DWORD VolumeSerial;                     //Số sector trong Volume
    BYTE VolumeLabel[11];
    BYTE FATID[8];
    BYTE BootProgram[420];                  //Sector chứa bản sao của Bootsector (offset 32 - 2 bytes)
    WORD EndSignature;
};
//
struct DIRECTORY {
    char Name[256];             // Tên thư mục/ tập tin
    int Attr;                   // Thuộc tính (thường là thư mục/ tập tin)
    int StartCluster;           // Cluster bắt đầu
    int FileSize;               // Kích cỡ (tính theo byte)
    DIRECTORY* next;            // Trỏ đến thư mục/ tập tin tiếp theo
    DIRECTORY* dir;             // Trỏ đến thư mục con
};
//
int ReadBootSectorFAT32(LPCWSTR  drive, int readPoint, BYTE sector[512]);
void InforBootSector(BOOTSECTORFAT32 fat32);
void covertSectorToBS32(BYTE sector[512], BOOTSECTORFAT32& fat32);

void initFAT(int*& FAT, BOOTSECTORFAT32 fat32, LPCWSTR drive1);
int firstSectorIndexOfCluster(int N, BOOTSECTORFAT32 fat32);

DIRECTORY* readDirectory(int firstRecordIndex, int clusIndex, int* entryList, BOOTSECTORFAT32 fat32, LPCWSTR drive1, string space);
void readContentOfFile(BOOTSECTORFAT32 fat32, int clusIndex, LPCWSTR drive1, string space);
void freeDirEntries(DIRECTORY* dir);