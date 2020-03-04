
#ifndef RANDOMC_H
#define RANDOMC_H

// Define integer types with known size: int32_t, uint32_t, int64_t, uint64_t.
// If this doesn't work then insert compiler-specific definitions here:
#if defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER >= 1600)
  // Compilers supporting C99 or C++0x have stdint.h defining these integer types
#include <stdint.h>
#include <cstdlib>
#define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#elif defined(_WIN16) || defined(__MSDOS__) || defined(_MSDOS) 
  // 16 bit systems use long int for 32 bit integer.
typedef   signed long int int32_t;
typedef unsigned long int uint32_t;
#elif defined(_MSC_VER)
  // Older Microsoft compilers have their own definition
typedef   signed __int32  int32_t;
typedef unsigned __int32 uint32_t;
typedef   signed __int64  int64_t;
typedef unsigned __int64 uint64_t;
#define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#else
  // This works with most compilers
typedef signed int          int32_t;
typedef unsigned int       uint32_t;
typedef long long           int64_t;
typedef unsigned long long uint64_t;
#define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#endif
class CRandomMother {                  // Encapsulate random number generator
public:
	void RandomInit(int seed);          // Initialization
	unsigned long long int IRandom(unsigned long long int min, unsigned long long int max);      // Get integer random number in desired interval
	double URandom();                    // Get floating point random number
	uint32_t BRandom();                 // Output random bits
	CRandomMother(void) {           // Constructor
		RandomInit(std::rand());
	}
protected:
	uint32_t x[5];                      // History buffer
};

class CRandomNormal {
public:
	double NRandom(double M, double O);
	CRandomNormal(void);
protected:
	int lust;
	double N[2] = { 0,0 };
	CRandomMother IIU0 = CRandomMother();
	CRandomMother IIU1 = CRandomMother();
};

class CRandomExp {
public:
	double ExpRandom(double M);
	CRandomExp(void);         // Constructor
protected:
	CRandomMother IIU0 = CRandomMother();
};

#endif