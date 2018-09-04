#pragma once
#pragma warning(disable:4702)
#pragma warning(disable:4100)
#pragma warning(disable:4201)
#pragma warning(disable:4511)
#pragma warning(disable:4663)
#pragma warning(disable:4018)
#pragma warning(disable:4245)
#pragma warning(disable:4710)	// not inlined
#pragma warning(disable:4786)	// character 255 넘어가는거 끄기
#pragma warning(disable:4244)	// type conversion possible lose of data

#pragma warning(disable:4018)
#pragma warning(disable:4245)
#pragma warning(disable:4512)
#pragma warning(disable:4201)
#include <string>
#include <stdlib.h>



class CTraduction
{
	public:
		std::string GetTraduction(std::string mot, std::string langueSource, std::string langueDest);

		
};

