#include <stdlib.h>
#include "buffer.h"
#include "system.h"

/* Device header file */
#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__dsPIC33E__)
    	#include <p33Exxxx.h>
    #elif defined(__dsPIC33F__)
    	#include <p33Fxxxx.h>
    #endif
#endif

//*** buffer_available *******************************************************//

uint16 buffer_available (buffer_t* buf)

{ // buffer_available
  int16 difference = (int16) buf->in_ptr - (int16) buf->out_ptr;

  return ((difference < 0) ? (difference + buf->ptr_max + 1) : difference);
} // buffer_available


//*** buffer_clear ***********************************************************//

void buffer_clear (buffer_t* buf)

{ // buffer_clear
  buf->in_ptr = buf->out_ptr;
} // buffer_clear


//*** buffer_examine *********************************************************//

boolean buffer_examine (buffer_t* buf, void* out_data)

{ // buffer_examine
  INTERRUPT_DISABLE; // don't allow interrupts, which could cause data inconsistency
  
  boolean examine_ok = TRUE;
  
  if (buf->data == NULL)
    { // bad buffer
      examine_ok = FALSE;
    } // bad buffer
  else if (buffer_is_empty (buf))
    { // there's nothing in the buffer
      examine_ok = FALSE;
    } // there's nothing in the buffer
  else
    { // examine the data from the buffer
      uint16 offset = buf->out_ptr * buf->element_size;
      
      uint8 byte_number;
      for (byte_number = 0; (byte_number < buf->element_size); byte_number++)
        { // examine each byte
          *((uint8*) (out_data + byte_number)) = *((uint8*) (buf->data + offset + byte_number));
        } // examine each byte
    } // examine the data from the buffer
  
  if (!examine_ok)
    { // something wrong, make return data zero
      uint8 byte_number;
      for (byte_number = 0; (byte_number < buf->element_size); byte_number++)
        { // zero each byte
          *((uint8*) (out_data + byte_number)) = 0;
        } // zero each byte
    } // something wrong, make return data zero
  
  INTERRUPT_REENABLE;
  
  return (examine_ok);
} // buffer_examine


//*** buffer_get *************************************************************//

boolean buffer_get (buffer_t* buf, void* out_data)

{ // buffer_get

  INTERRUPT_DISABLE; // don't allow interrupts, which could cause data inconsistency
  
   boolean get_ok = buffer_examine (buf, out_data);
  
  if (get_ok)
    buf->out_ptr = ((buf->out_ptr >= buf->ptr_max) ? 0 : (buf->out_ptr + 1)); // update the pointer
  
  INTERRUPT_REENABLE;
  
  return (get_ok);
} // buffer_get


//*** buffer_initialize ******************************************************//


boolean buffer_initialize (buffer_t* buf, void *bufptr, uint8 element_size, uint16 buffer_size)
{ // buffer_initialize
  buf->in_ptr       = 0;
  buf->out_ptr      = 0;
  buf->ptr_max      = buffer_size - 1;
  buf->element_size = element_size;
  
  buf->data = bufptr; //malloc (buffer_size * element_size);
  
  return ((buf->data == NULL) ? FALSE : TRUE);
} // buffer_initialize


//*** buffer_is_empty ********************************************************//

boolean buffer_is_empty (buffer_t* buf)

{ // buffer_is_empty
  return ((buf->in_ptr == buf->out_ptr) ? TRUE : FALSE);
} // buffer_is_empty


//*** buffer_is_full *********************************************************//

boolean buffer_is_full (buffer_t* buf)

{ // buffer_is_full
  return ((((buf->in_ptr >= buf->ptr_max) ? 0 : (buf->in_ptr + 1)) == buf->out_ptr)
           ? TRUE : FALSE);
} // buffer_is_full


//*** buffer_put *************************************************************//

boolean buffer_put (buffer_t* buf, void* in_data)

{ // buffer_put
  INTERRUPT_DISABLE; // don't allow interrupts, which could cause data inconsistency

  boolean put_ok = TRUE;
  
  if (buf->data == NULL)
    { // bad buffer
      put_ok = FALSE;
    } // bad buffer
  else if (buffer_is_full (buf))
    { // there's no room left
      put_ok = FALSE;
    } // there's no room left
  else
    { // put the data into the buffer
      uint16 offset = buf->in_ptr * buf->element_size;
      
      uint8 byte_number;
      for (byte_number = 0; (byte_number < buf->element_size); byte_number++)
        { // put in each byte
          *((uint8*) (buf->data + offset + byte_number)) = *((uint8*) (in_data + byte_number));
        } // put in each byte
      
      buf->in_ptr = ((buf->in_ptr >= buf->ptr_max) ? 0 : (buf->in_ptr + 1));
    } // put the data into the buffer
  
  INTERRUPT_REENABLE;
  
  return (put_ok);
} // buffer_put


//*** buffer_size ************************************************************//

uint16  buffer_size (buffer_t* buf)

{ // buffer_size
  return (buf->ptr_max + 1);
} // buffer_size

