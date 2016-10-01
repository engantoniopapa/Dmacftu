#include <linux/circular_buffer.h>
#include <linux/module.h>

/* alloc and init an instance of struct CircularBuffer */
 void cbInit(struct CircularBuffer *cb )
{
	cb->size = CIRC_BUFF_SIZE;
	cb->array_element =(struct unit_buff *) kzalloc(sizeof(struct unit_buff )* (cb->size) , GFP_KERNEL);
	cb->start = 0;
	cb->end   = 0;
	cb->count_event = 0;
}
EXPORT_SYMBOL_GPL(cbInit);

/* free an instance of struct CircularBuffer */
void cbFree(struct CircularBuffer *cb) 
{
    kfree(cb->array_element); /* OK if null */ 
}
EXPORT_SYMBOL_GPL(cbFree);
 
/* Write an element, not overwriting oldest element if buffer is full (wine policy).  */
void cbWrite(struct CircularBuffer *cb, struct unit_buff *elem) 
{
	if( !cbIsFull(cb) )
	{
		memcpy(&(cb->array_element[cb->end]) , elem , sizeof(struct unit_buff));
		mb();
		cb->end = (cb->end + 1) % cb->size;
	} /* full, no overwrite */
}
EXPORT_SYMBOL_GPL(cbWrite);

/* Read oldest element and return a its copy. App must ensure !cbIsEmpty() first. */
void cbRead(struct CircularBuffer *cb, struct unit_buff *elem) 
{
	memcpy(elem, &(cb->array_element[cb->start]) , sizeof(struct unit_buff));
	mb();
  cb->start = (cb->start + 1) % cb->size;
}
EXPORT_SYMBOL_GPL(cbRead);

/* Read at most @size oldest element and returns the number of elements copied */
int cbRead_list(struct CircularBuffer *cb, struct unit_buff *elem , int size) 
{
	int segment_1 = cb->end;
	int segment_2 = 0;
	
	if(segment_1 == cb->start)
		return 0;
	
	if(segment_1 < cb->start)
	{		
		segment_2 = segment_1;
		segment_1 = cb->size - cb->start;
		
		if((segment_1+segment_2) > size )  
		{
			if(segment_1 > size)
			{
				segment_1 = size;
				segment_2 = 0;
			}
			else
			{
				segment_2 = size - segment_1;
			}
		}
	}
	else
	{
		segment_1 = segment_1 - cb->start ;
		
		if(segment_1 > size )  
			segment_1 = size;
	}
	
	memcpy(elem, &(cb->array_element[cb->start]) , sizeof(struct unit_buff)*segment_1);
	mb();
  cb->start = (cb->start + segment_1) % cb->size;
  mb();

  if(segment_2)
  {
		memcpy(&(elem[segment_1]), &(cb->array_element[cb->start]) , sizeof(struct unit_buff)*segment_2);
		mb();
		cb->start = (cb->start + segment_2) % cb->size;
	}
  
  return (segment_1 + segment_2);
}
EXPORT_SYMBOL_GPL(cbRead_list);

