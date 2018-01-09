#include "Serializer.h"
#include "Deserializer.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>

int main(int argc, char** argv)
{
	std::cout << "LittleEndian test:" << std::endl;
	ara::com::Serializer sl(ara::com::ByteOrderEnum::LittleEndian);
	
	ara::com::boolean b1 = false;
	ara::com::boolean b2 = true;
	ara::com::float64 f = -6553665536.589109;
	ara::com::float32 f32 = -65536.589;
	ara::com::float32 f_t = -65536.589;
	ara::com::sint64 si = -123456789123456789;
	
	sl.serialize(b1);
	sl.serialize(b2); 
	sl.serialize(f);
	sl.serialize(f32);
	sl.serialize(si);
	
	const uint8_t *data = sl.getData();
	uint32_t size = sl.getSize();
	
	ara::com::Deserializer dsl(ara::com::ByteOrderEnum::LittleEndian, data, size);
	
	b1 = true;
	b2 = false;
	f = 0;
	f32 = 0;
	si = 0;
	
	dsl.deserialize(b1);
	dsl.deserialize(b2);
	dsl.deserialize(f);
	dsl.deserialize(f32);
	dsl.deserialize(si);
	
	std::cout << "b1=" << (b1 ? "true" : "false") << std::endl;
	std::cout << "b2=" << (b2 ? "true" : "false") << std::endl;
	
	std::cout << setiosflags(std::ios::fixed);
	std::cout << std::setprecision(6) << f << std::endl;
	std::cout << std::setprecision(3) << f32 << std::endl;
	std::cout << std::setprecision(3) << f_t << std::endl;
	std::cout << si << std::endl;
	
	std::cout << "BigEndian test:" << std::endl;
	
	ara::com::Serializer sb(ara::com::ByteOrderEnum::BigEndian);
	
	f = -6553665536.589109;
	si = -123456789123456789; 
	sb.serialize(f);
	sb.serialize(si);
	
	data = sb.getData();
	size = sb.getSize();
	
	ara::com::Deserializer dsb(ara::com::ByteOrderEnum::BigEndian, data, size);
	
	f = 0;
	si = 0;
	
	dsb.deserialize(f);
	dsb.deserialize(si);
	
	std::cout << setiosflags(std::ios::fixed);
	
	std::cout << std::setprecision(6) << f << std::endl;
	std::cout << si << std::endl;

	return 0;
}
