#include "duktape.h"

static int32_t net_next_id = 1;

static duk_ret_t net_createConnection(duk_context *ctx) {
  return 0;
}

static duk_ret_t net_serverListen(duk_context *ctx) {
  printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
  return 0;
}

static duk_ret_t net_serverClose(duk_context *ctx) {
  printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
  return 0;
}

static duk_ret_t net_serverAddress(duk_context *ctx) {
  printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
  return 0;
}

static duk_ret_t net_serverFinalizer(duk_context *ctx) {
  printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
  return 0;
}

const duk_function_list_entry net_server_functions_list[] = {
  { "listen", net_serverListen, 4  },
  { "close", net_serverClose, 4  },
  { "address", net_serverAddress, 0  },
  { NULL, NULL, 0 }
};

static duk_ret_t net_createServer(duk_context *ctx) {
  int32_t timer_id = net_next_id++;
  duk_int_t p0 = duk_get_type(ctx, 0);
  duk_int_t p1 = duk_get_type(ctx, 1);

  if (duk_is_object(ctx, 0) && duk_is_callable(ctx, 0)) { // Function
    printf("Function\n");
    duk_push_this(ctx); // [ this ]
    duk_push_object(ctx); // [ object object ]
    duk_put_prop_string(ctx, -2, "\xff""\xff""options"); // [ object.options ]
    duk_dup(ctx, 0); // [ object.options function ]
    duk_put_prop_string(ctx, -2, "\xff""\xff""callback"); // [ netobject ]
    duk_put_function_list(ctx, -1, net_server_functions_list);
    duk_push_c_function(ctx, net_serverFinalizer, 1);
    duk_set_finalizer(ctx, -2);

    return 1;
  } else if (duk_is_object(ctx, 0) && !duk_is_callable(ctx, 0) &&
    duk_is_object(ctx, 1) && duk_is_callable(ctx, 1)) { // Object , Function
    printf("Function & Obj\n");

    duk_push_this(ctx); // [ object ]
    duk_dup(ctx, 0); // [ object object ]
    duk_put_prop_string(ctx, -2, "\xff""\xff""options"); // [ object.options ]
    duk_dup(ctx, 1); // [ object.options function ]
    duk_put_prop_string(ctx, -2, "\xff""\xff""callback"); // [ netobject ]
    duk_put_function_list(ctx, -1, net_server_functions_list);
    duk_push_c_function(ctx, net_serverFinalizer, 1);
    duk_set_finalizer(ctx, -2);

    return 1;
  } else {
    return DUK_RET_ERROR;
  }

  return 0;
}

static duk_ret_t net_connect(duk_context *ctx) {
  printf("%s %d - Top : %d\n", __PRETTY_FUNCTION__, __LINE__, duk_get_top(ctx));
  return 0;
}

const duk_function_list_entry net_functions_list[] = {
  { "createServer", net_createServer, 2 },
  { "connect", net_connect, 3 },
  { "createConnection", net_createConnection, 3 },
  { NULL, NULL, 0}
};

void net_Init(duk_context *ctx) {
  duk_push_global_object(ctx); // [ global ]
  duk_push_object(ctx);        // [ global obj ]
  duk_put_function_list(ctx, -1, net_functions_list); // [ global obj.functions ]
  duk_put_prop_string(ctx, -2, "net"); // [ global.net.functions ]
  duk_pop(ctx);
}

int main(int argc, char **argv) {
  duk_context *ctx = duk_create_heap_default();

  net_Init(ctx);
  duk_eval_file(ctx, "tests/net_test.js");

  duk_destroy_heap(ctx);

  return 0;
}