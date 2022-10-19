#include"FAT32.h"
#include"Util.h"
//Ham phu tro
unsigned int reversedWORD(WORD buffer) {
	unsigned int result = 0;
	BYTE arr[2];
	memcpy(arr, &buffer, 2);
	result = (result << 8) | arr[1];
	result = (result << 8) | arr[0];
	return result;
}
//
unsigned int reversedDWORD(DWORD buffer) {
	unsigned int result = 0;
	BYTE arr[4];
	memcpy(arr, &buffer, 4);
	for (int i = 3; i >= 0; i--)
		result = (result << 8) | arr[i];
	return result;
}

//--------------------------
void covertSectorToBS32(BYTE sector[512], BOOTSECTORFAT32& bs32) {
	memset(&bs32, 0, 512);

	memcpy(&bs32.JUMP, sector, sizeof(bs32.JUMP));
	memcpy(&bs32.OEM, sector + 3, sizeof(bs32.OEM));
	memcpy(&bs32.BytePerSector, sector + 11, sizeof(bs32.BytePerSector));
	memcpy(&bs32.SectorPerCluster, sector + 13, sizeof(bs32.SectorPerCluster));
	memcpy(&bs32.ReservedSector, sector + 14, sizeof(bs32.ReservedSector));
	memcpy(&bs32.FatNum, sector + 16, sizeof(bs32.FatNum));
	memcpy(&bs32.EntryRDET, sector + 17, sizeof(bs32.EntryRDET));
	memcpy(&bs32.LowNumberSectors, sector + 19, sizeof(bs32.LowNumberSectors));
	memcpy(&bs32.DeviceType, sector + 21, sizeof(bs32.DeviceType));
	memcpy(&bs32.SectorPerFat16, sector + 22, sizeof(bs32.SectorPerFat16));
	memcpy(&bs32.SectorPerTrack, sector + 24, sizeof(bs32.SectorPerTrack));
	memcpy(&bs32.HeadPerDisk, sector + 26, sizeof(bs32.HeadPerDisk));

	memcpy(&bs32.NumberHiddenSectors, sector + 28, sizeof(bs32.NumberHiddenSectors));
	memcpy(&bs32.HighNumberSectors, sector + 32, sizeof(bs32.HighNumberSectors));
	memcpy(&bs32.SectorPerFat32, sector + 36, sizeof(bs32.SectorPerFat32));
	memcpy(&bs32.Bit8Flag, sector + 40, sizeof(bs32.Bit8Flag));
	memcpy(&bs32.FAT32Ver, sector + 42, sizeof(bs32.FAT32Ver));
	memcpy(&bs32.FirstRDETCluster, sector + 44, sizeof(bs32.FirstRDETCluster));
	memcpy(&bs32.AddiInfoSector, sector + 48, sizeof(bs32.AddiInfoSector));
	memcpy(&bs32.BackupSector, sector + 50, sizeof(bs32.BackupSector));
	memcpy(&bs32.LaterVerReserved, sector + 52, sizeof(bs32.LaterVerReserved));

	memcpy(&bs32.PhysicDisk, sector + 64, sizeof(bs32.PhysicDisk));
	memcpy(&bs32.Reserved, sector + 65, sizeof(bs32.Reserved));
	memcpy(&bs32.Signature, sector + 66, sizeof(bs32.Signature));
	memcpy(&bs32.VolumeSerial, sector + 67, sizeof(bs32.VolumeSerial));
	memcpy(&bs32.VolumeLabel, sector + 71, sizeof(bs32.VolumeLabel));
	memcpy(&bs32.FATID, sector + 82, sizeof(bs32.FATID));
	memcpy(&bs32.BootProgram, sector + 90, sizeof(bs32.BootProgram));
	memcpy(&bs32.EndSignature, sector + 510, sizeof(bs32.EndSignature));
}
//
int ReadBootSectorFAT32(LPCWSTR  drive, int readPoint, BYTE sector[512]) {
	int retCode = 0;
	DWORD bytesRead;
	HANDLE device = NULL;
	//don't care
	device = CreateFile(drive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (device == INVALID_HANDLE_VALUE) {
		printf("CreateFile: %u\n", GetLastError());
		return 1;
	}
	SetFilePointer(device, readPoint, NULL, FILE_BEGIN);    //Set a Point to Read
	if (!ReadFile(device, sector, 512, &bytesRead, NULL)) {
		printf("ReadFile: %u\n", GetLastError());
		return 1;
	}
	else {
		//printf("success!");
	}
	CloseHandle(device);
	return 0;
}
//
void InforBootSector(BOOTSECTORFAT32 bs32) {
	int Sb = reversedWORD(bs32.ReservedSector);//So sector truoc bang FAT
	int Nf = reversedWORD(bs32.FatNum); //so bang FAT
	int Sf = reversedDWORD(bs32.SectorPerFat32);//so sector tren 1 bang FAT32
	cout << "1. OEM name/version: " << bs32.OEM << endl;
	cout << "2. Byte Per Sector: " << reversedWORD(bs32.BytePerSector) << endl;

	//byte/sector  *sector/cluter
	cout << "3. Sector per cluster: " << reversedWORD(bs32.SectorPerCluster) << endl;
	cout << "4. Reserved sectors: " << Sb << endl;
	cout << "5. FAT copies: " << Nf << endl;
	cout << "6. Sector in Volume: " << reversedDWORD(bs32.VolumeSerial) << endl;
	cout << "7. Sector per FAT: " << reversedDWORD(bs32.SectorPerFat32) << endl;
	cout << "8. Sector dau tien cua bang FAT: " << Sb << endl;
	cout << "9. Sector dau tien cua bang RDET: " << Sb + Nf * Sf << endl;
}
//
void initFAT(int*& FAT, BOOTSECTORFAT32 fat32, LPCWSTR drive1)
{
	//There are 2 FAT tables
	//FAT32 -- 32 bits (4 Bytes) but only 28 bits are used and high 4 bits are reserved	
	int fatTableSize = fat32.SectorPerFat32 * fat32.BytePerSector;
	BYTE* readBytes = new BYTE[fatTableSize];
	fstream diskStream(drive1, std::ios::in);
	char* a = new char[fatTableSize];
	diskStream.seekg(fat32.ReservedSector * fat32.BytePerSector, 0);
	diskStream.read(a, fatTableSize);

	readBytes = (BYTE*)a;
	int numberOfFATEntries = fatTableSize / 4; //FAT_RECORD_SIZE;
	int count = 0;
	//8946
	for (int i = 0; i < numberOfFATEntries; i++)
	{
		if (hexToDecimal(readBytes, 4 * i, 4) == 0)
		{
			count += 1;
			if (count >= 200)
				break;
		}
		FAT[i] = hexToDecimal(readBytes, 4 * i, 4);
	}
}
//
int firstSectorIndexOfCluster(int clusIndex, BOOTSECTORFAT32 fat32)
{
	int dataFirstSector = fat32.ReservedSector + fat32.FatNum * fat32.SectorPerFat32;
	return ((clusIndex - 2) * fat32.SectorPerCluster) + dataFirstSector;
}
//
DIRECTORY* readDirectory(int firstEntryIndex, int clusIndex, int* FAT, BOOTSECTORFAT32 fat32, LPCWSTR drive1, string space)
{
	int clusSize = fat32.SectorPerCluster * fat32.BytePerSector;
	//Root directory entries có kích thước 32 Bytes 
	//và số lượng các entries bị giới hạn bởi kích thước của cluster
	//và kích thước dựa trên FAT entries

	int totalEntries = (fat32.SectorPerCluster * fat32.BytePerSector) / 32;
	char* a = new char[clusSize];
	int firstSector = firstSectorIndexOfCluster(clusIndex, fat32);
	fstream diskStream(drive1, std::ios::in);

	diskStream.seekg(firstSector * fat32.BytePerSector, SEEK_SET);
	diskStream.read(a, clusSize);
	BYTE* readBytes = new BYTE[clusSize];
	readBytes = (BYTE*)a;

	//đọc từng entry
	DIRECTORY* dirHeadNode, * dirTempNode, * dirTailNode;
	dirHeadNode = dirTempNode = dirTailNode = NULL;

	//holder to store longer named
	int byteHolder[13];
	int name[256];
	int holderIndex = -1;
	int nameIndex = -1;

	for (int i = firstEntryIndex; i < totalEntries; i++)
	{
		//cuối bảng ghi
		if (readBytes[32 * i] == 0x0) break;

		//tập tin không được sử dụng do bị xóa
		if (readBytes[32 * i] == 0xE5) continue;

		int status = hexToDecimal(32 * i + readBytes, 11, 1);

		//LONG FILE NAME ENTRY
		if (status == 0xF)
		{
			//Code để lấy được Long file name, đọc lần lượt 2 Bytes để lấy cái kí tự
			//1 -- 10
			for (int j = 1; j < 10; j += 2)
			{
				if (hexToDecimal(32 * i + readBytes, j, 2) == 0xffff) break; //kết thúc file name
				byteHolder[++holderIndex] = hexToDecimal(32 * i + readBytes, j, 2);
			}
			//14 -- 25
			for (int j = 14; j < 25; j += 2)
			{
				if (hexToDecimal(32 * i + readBytes, j, 2) == 0xffff) break; //kết thúc file name
				byteHolder[++holderIndex] = hexToDecimal(32 * i + readBytes, j, 2);
			}
			//28 -- 31
			for (int j = 28; j < 31; j += 2)
			{
				if (hexToDecimal(32 * i + readBytes, j, 2) == 0xffff) break; //kết thúc file name
				byteHolder[++holderIndex] = hexToDecimal(32 * i + readBytes, j, 2);
			}
			//POP VÀ PUSH VÀO LONG NAME STACK
			while (holderIndex != -1)
				name[++nameIndex] = byteHolder[holderIndex--];
			continue; //Bởi vì Long file name entry được xác định nên bỏ qua
		}
		//Chỉ có các tập tin và thư mục được xét
		//và các tập tin liên quan đến hệ thống được bỏ qua dựa vào Attr

		if (status == 0x10 || status == 0x20)
		{
			dirTempNode = new DIRECTORY;
			dirTempNode->next = dirTempNode->dir = NULL;

			if (dirHeadNode == NULL)
				dirHeadNode = dirTempNode;
			else
				dirTailNode->next = dirTempNode;

			dirTailNode = dirTempNode;

			int count = 0; //sử dụng để đếm các ký tự
			for (int j = 0; j < 11; j++)
				dirTempNode->Name[count++] = readBytes[32 * i + j];

			if (nameIndex != -1)
			{
				count = 0;
				for (int j = nameIndex; j > -1; j--)
					dirTempNode->Name[count++] = (char)name[j];

				holderIndex = nameIndex = -1;
			}
			dirTempNode->Name[count] = '\0';
			dirTempNode->Attr = status;
			dirTempNode->FileSize = hexToDecimal(32 * i + readBytes, 28, 4);
			dirTempNode->StartCluster = (hexToDecimal(32 * i + readBytes, 20, 2)
				* pow(2, 16)) + hexToDecimal(32 * i + readBytes, 26, 2);


			//to read directories in directories

			cout << space << "| " << "Ten file: " << dirTempNode->Name << space << space << (dirTempNode->Attr == 0x10 ? "<Directory>" : "<Archive>") << " |" << endl;
			cout << space << "| " << "Kich thuoc: " << dirTempNode->FileSize << " B" << " |" << endl;
			cout << space << "| " << "Cluster bat dau: " << dirTempNode->StartCluster << " |" << endl;
			if (status == 0x10)
			{
				space = space + "     ";
				cout << endl;
				dirTempNode->dir = readDirectory(2, dirTempNode->StartCluster, FAT, fat32, drive1, space); //2 to skip . and .. dir entries
				space = space.substr(0, space.length() - 5);
			}
			else if (status == 0x20)
			{
				int len = strlen(dirTempNode->Name) - 3;
				char string[4];
				strcpy(string, dirTempNode->Name + len);

				if (strcmp(string, "txt") == 0 || strcmp(string, "TXT") == 0)
				{
					cout << space << "| " << "Noi dung file TXT: ";
					readContentOfFile(fat32, dirTempNode->StartCluster, drive1, space);
				}
				else
					cout << space << "--> " << "Dung phan mem tuong thich de doc noi dung!" << endl;
				cout << "-------------------------------------------------------------------" << endl;
			}

			if (FAT[clusIndex] != 0x0fffffff)
				dirTailNode->next = readDirectory(firstEntryIndex, FAT[clusIndex], FAT, fat32, drive1, space);
		}
	}

	return dirHeadNode;
}
//
void readContentOfFile(BOOTSECTORFAT32 fat32, int clusIndex, LPCWSTR drive1, string space) {
	int content_sector = firstSectorIndexOfCluster(clusIndex, fat32);
	int clusterSizeInBytes = fat32.SectorPerCluster * fat32.BytePerSector;

	char* a = new char[clusterSizeInBytes];
	fstream G(drive1, std::ios::in);
	G.seekg(content_sector * fat32.BytePerSector, 0);
	G.read(a, clusterSizeInBytes);

	char* content = new char[clusterSizeInBytes];
	int i = 0;
	while (a[i] != '\x00')
	{
		content[i] = a[i];
		i++;
		if (i == 4095) break;
	}
	content[i] = '\0';

	cout << space << content << endl;
}

void freeDirEntries(DIRECTORY* dir)
{
	DIRECTORY* temp;
	while (dir)
	{
		if (dir->dir)
			freeDirEntries(dir->dir);

		temp = dir;
		dir = dir->next;
		delete temp;
	}
}


