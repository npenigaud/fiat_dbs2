#include <stdlib.h>
#include <stddef.h>

#include <stdio.h>
void * alloc_aligned(size_t alignment, size_t size_bytes)
{
  void * buffer;
// printf("before memalign, size_bytes= %zu alignement= %zu",size_bytes,alignment);
  posix_memalign(&buffer,alignment, size_bytes);
//  return aligned_alloc(alignment, size_bytes); can be aligned on less than size(void*)
//  printf("after memalign");
  return buffer; 
}

void free_aligned(void ** buffer)
{
  free(*buffer);
}
