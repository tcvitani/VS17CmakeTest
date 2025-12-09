#ifndef TLM_DMV_H
#define TLM_DMV_H

typedef enum _enum_tlm_dmv_images
{
	TLM_DMV_FIRST = 0,
	TLM_DMV_IMAGE_FULL_GREEN = TLM_DMV_FIRST,
	TLM_DMV_IMAGE_FULL_RED = 1,
	TLM_DMV_IMAGE_GREEN_ARROW = 2,
	TLM_DMV_IMAGE_RED_CROSS = 3,
	TLM_DMV_LAST = TLM_DMV_IMAGE_RED_CROSS,
}
enum_tlm_dmv_images;

#endif