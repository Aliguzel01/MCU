

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
 #include "stm32f407xx.h"
#include<stdint.h>



int main(void)
{

	return 0;
}
