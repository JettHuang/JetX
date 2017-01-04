// \brief
//		some utility functions 
//

#ifndef __JETX_UTILITY_HELPER_H__
#define __JETX_UTILITY_HELPER_H__

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#ifndef MIN
#define	MIN(a, b)	( (a)>(b) ? (b) : (a) )
#endif
#ifndef MAX
#define MAX(a, b)	( (a)>(b) ? (a) : (b) )
#endif

// class Helper
class FUtilityHelper
{
public:
	
	// read a text file
	static std::string ReadTextFile(const char *InFileName);
};


#define STRUCT_VAR_OFFSET(cls, var)		((char *)&(((cls*)0)->var) - ((char *)0))

#endif // __JETX_UTILITY_HELPERS_H__
