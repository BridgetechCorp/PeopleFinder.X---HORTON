#ifndef BUFFER_H
#define BUFFER_H

#include "types.h"

typedef struct
{ // buffer_t
  uint16 in_ptr;
  uint16 out_ptr;
  uint16 ptr_max;
  uint16 element_size;
  void*  data;  
} // buffer_t
buffer_t;


uint16  buffer_available  (buffer_t* buf);  // return the number of elements in the buffer
void    buffer_clear      (buffer_t* buf);  // clear the buffer (make it empty)
boolean buffer_examine    (buffer_t* buf, void* out_data);  // TRUE if data is available
boolean buffer_get        (buffer_t* buf, void* out_data);  // TRUE if data is retrieved
boolean buffer_initialize (buffer_t* buf, void *bufptr, uint8 element_size, uint16 buffer_size);  // TRUE if buffer is empty
boolean buffer_is_empty   (buffer_t* buf);  // TRUE if buffer is empty
boolean buffer_is_full    (buffer_t* buf);  // TRUE if buffer is full
boolean buffer_put        (buffer_t* buf, void* in_data);  // TRUE if data is put
uint16  buffer_size       (buffer_t* buf);  // return the size of the buffer (number of elements)

#endif // Sentry
