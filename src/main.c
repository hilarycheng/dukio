#include <stdio.h>
#include "duktape.h"
#include "uv.h"

int main(int argc, char **argv)
{
  uv_loop_t *loop = uv_default_loop();

  duk_context *ctx = duk_create_heap_default();
  duk_eval_string(ctx, "print('Hello world!');");
  duk_destroy_heap(ctx);

  return 0;
}
