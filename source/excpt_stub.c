#include <calico.h>
#include <nds.h>

#include "nds_loader_arm9.h"
#include "exceptionstub_bin.h"

typedef struct excptStub_t {
	u32 dummy;
	void* bss_start;
	void* bss_end;
	VoidFn excpt_vector;
} excptStub_t;

void installExcptStub(void)
{
	excptStub_t* exceptionstub = (excptStub_t*)0x2ffa000;
	armCopyMem32(exceptionstub,exceptionstub_bin,exceptionstub_bin_size);
	if (exceptionstub->bss_start != exceptionstub->bss_end) {
		armFillMem32(exceptionstub->bss_start, 0, (char*)exceptionstub->bss_end - (char*)exceptionstub->bss_start);
	}

	EXCEPTION_VECTOR = exceptionstub->excpt_vector;
}
