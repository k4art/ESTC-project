#include "xbutton.h"

#include "internals/btn_clickable.h"

void xbutton_init(void)
{
  btn_clickable_init();
}

nrfx_err_t xbutton_enable(uint8_t button_idx, bool high_accuracy)
{
  return btn_clickable_enable(button_idx, high_accuracy);
}

void xbutton_on_click(uint8_t button_idx, xbutton_handler_t handler)
{
  btn_clickable_on_click(button_idx, handler);
}
