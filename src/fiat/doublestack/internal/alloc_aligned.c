#include <stdlib.h>
#include <stddef.h>

#include <stdio.h>
void * alloc_aligned(size_t alignment, size_t size_bytes)
{
  void * buffer;
  posix_memalign(&buffer,alignment, size_bytes);
  return buffer; 
}

void free_aligned(void ** buffer)
{
  free(*buffer);
}
