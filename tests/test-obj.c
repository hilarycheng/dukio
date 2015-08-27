#include "duktape.h"

static duk_ret_t do_callback(duk_context *ctx)
{
  duk_bool_t rc;
  printf("%s %d Constructor : %d\n", __PRETTY_FUNCTION__, __LINE__, duk_is_constructor_call(ctx));
//  duk_push_string(ctx, "Constructor from testObj");
//  duk_put_prop_string(ctx, -1, "Hello");

printf("stack top is %ld\n", (long) duk_get_top(ctx));


  return 0;
}

const duk_function_list_entry test_module_funcs[] = {
    { "testobj", do_callback, 0 },
    { NULL, NULL, 0 }
};

int main(int argc, char **argv)
{
  duk_context *ctx = duk_create_heap_default();

  duk_push_global_object(ctx);              // Global
  duk_push_c_function(ctx, do_callback, 0); // Cons
  duk_put_prop_string(ctx, -2, "testobj"); 
//  duk_push_global_object(ctx);
//  duk_put_function_list(ctx, -1, test_module_funcs);
//  duk_pop(ctx);

  duk_eval_file(ctx, "test-obj.js");
  duk_destroy_heap(ctx);

  return 0;
}
