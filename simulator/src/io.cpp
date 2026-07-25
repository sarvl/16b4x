#include "io.h"
#include "privilege.h"
#include "interrupt.h"
#include "memory.h"

#include <fstream>
#include <string>

#include <cstdio>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

char* disk_data = nullptr;
uint32_t disk_size = 0;
uint32_t disk_addr = 0;

constexpr int hexd_to_val(char const hexd)
{
	constexpr signed char translate[128] = {
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	return translate[hexd & 0x7F];
}

#include "state.h"

uint16_t drive_read(uint32_t addr)
{
	if(addr >= disk_size)
	{
		printf("IP: %04X %d\n", ip, ip);
		printf("ERROR: trying to access drive line %d, but the drive has only %d\n words\n", addr, disk_size);
		return 0;
	}

	unsigned const data = 0
		| (hexd_to_val(disk_data[5 * addr + 0]) << 12)
		| (hexd_to_val(disk_data[5 * addr + 1]) <<  8)
		| (hexd_to_val(disk_data[5 * addr + 2]) <<  4)
		| (hexd_to_val(disk_data[5 * addr + 3]) <<  0)
		;
	
	if(data > 0xFFFF)
	{
		printf("ERROR: invalid data at drive line %d\n", addr);
		return 0;
	}
		
	return static_cast<uint16_t>(data);
}

void drive_write(uint32_t addr, uint16_t data)
{
	if(addr >= disk_size)
	{
		printf("ERROR: trying to access drive line %d, but the drive has only %d\n words\n", addr, disk_size);
		return;
	}
		
	printf("wrote %04X to %04X\n", data, addr);
	snprintf(disk_data + (5 * addr), 5, "%04X", data);
	disk_data[5 * addr + 4] = '\n';
	return;
}

uint16_t catalyst_info_return; 

uint16_t io::read (uint16_t const pid)
{
	if(not privilege::check(privilege::io_access))
	{
		interrupt::raise_prv(privilege::io_access);
		return 0;
	}

	uint16_t const device_id = pid >> 10;
	uint16_t const port      = pid & 0x03FF;


	switch(device_id)
	{
	case io::catalyst::id: switch(port)
	{
		using namespace io::catalyst;

		case port::cmd:
			return catalyst_info_return;

		case port::drive_addr:
			return static_cast<uint16_t>(disk_addr & 0xFFFF);
		
		case port::drive_data:
			return drive_read(disk_addr);

		default:
			return 0;
	}

	default:
		return 0x0000;
	}
}
void     io::write(uint16_t const pid, uint16_t const data)
{
	if(not privilege::check(privilege::io_access))
	{
		interrupt::raise_prv(privilege::io_access);
		return;
	}

	uint16_t const device_id   = pid >> 10;
	uint16_t const port        = pid & 0x3FF;

	switch(device_id)
	{
	case io::catalyst::id: switch(port)
	{
		using namespace io::catalyst;

		case port::cmd: switch(data >> 8)
		{
		default:
		case cmd::invalid:
			return;
		case cmd::info_fetch: switch(data & 0xFF)
		{
			default:
			case info_fetch::invalid:
				return;
			case info_fetch::version:
				catalyst_info_return = 0x0101;
				return;
			case info_fetch::pmemsize:
				catalyst_info_return = 0x0020;
				return;
		}

		case cmd::err_id:
			printf("error led: %06b\n", data & 0x3F);
			return;
		case cmd::io_init:
			//nothing to be done
			return;
		}

	case 0x000E:
		disk_addr = data;
		return;
	case 0x000F:
		disk_addr = data;
		return;
	}
	}

	return;
}

int      io::reset(char const* const filename)
{
	//the file lives for the whole duration of the program anyway
	//who cares about proper garbage collection amiright
	int const file_fd   = open(filename, O_RDONLY);
	int const file_size = lseek(file_fd, 0, SEEK_END);
	
	disk_data = (char*)mmap(NULL, file_size, PROT_READ, MAP_SHARED, file_fd, 0);
	disk_size = static_cast<uint32_t>(file_size / 5);
	
	std::ifstream read(filename);
	std::string line;

	uint32_t line_count = 0;
	while(line_count < disk_size && line_count < 512)
	{
		//this write requires modifying physical memory directly
		mem::write_pm(line_count, drive_read(line_count));
		line_count++;
	}

	return 0;
}
