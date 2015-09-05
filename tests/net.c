#include <uv.h>
#include "duktape.h"

static duk_context *mainContext;
static int32_t net_next_id = 1;
static int32_t sock_next_id = 1;

static void libuv_close(uv_handle_t *handle) {
  int32_t id = (int32_t) handle->data;
  free(handle);

  duk_push_global_stash(mainContext);
  duk_get_prop_string(mainContext, -1, "sockHandler");
  duk_push_int(mainContext, id);
  duk_get_prop(mainContext, -2);
  duk_bool_t is_undefined = duk_is_undefined(mainContext, -1);
  if (!is_undefined) {
      duk_push_string(mainContext, "close");
      duk_get_prop(mainContext, -2);
      duk_bool_t is_undefined = duk_is_undefined(mainContext, -1);
      if (!is_undefined) {
        duk_push_int(mainContext, 1);
        duk_pcall(mainContext, 1);
      }
      duk_pop(mainContext);
  }
  duk_pop_n(mainContext, 3);
  if (!is_undefined) {
    duk_push_global_stash(mainContext);
    duk_get_prop_string(mainContext, -1, "sockHandler");
    duk_push_int(mainContext, id);
    duk_del_prop(mainContext, -2);
    duk_pop_n(mainContext, 2);
  }
}

static void libuv_read_cb(uv_stream_t * stream, ssize_t nread, uv_buf_t buf) {
  int32_t id = (int32_t) stream->data;

  if (nread == -1) {
    uv_close((uv_handle_t *) stream, libuv_close);
  } else {
    duk_push_global_stash(mainContext);
    duk_get_prop_string(mainContext, -1, "sockHandler");
    duk_push_int(mainContext, id);
    duk_get_prop(mainContext, -2);
    duk_bool_t is_undefined = duk_is_undefined(mainContext, -1);

    if (!is_undefined) {
      duk_push_string(mainContext, "data");
      duk_get_prop(mainContext, -2);
      duk_bool_t is_undefined = duk_is_undefined(mainContext, -1);
      if (!is_undefined) {
        duk_push_int(mainContext, 1);
        duk_pcall(mainContext, 1);
      }
      duk_pop(mainContext);
    }

    duk_pop_n(mainContext, 3);
  }

  free(buf.base);
}

static uv_buf_t libuv_alloc_buffer(uv_handle_t * handle, size_t size) {
  return uv_buf_init((char *) malloc(size), size);
}

static duk_ret_t net_socketWrite(duk_context *ctx) {
  uv_tcp_t *tcp = NULL;
  int sock_id = 0;

  duk_push_this(ctx);
  duk_get_prop_string(ctx, -1, "\xff""\xff""handler");
  tcp = (uv_tcp_t *) duk_to_pointer(ctx, -1);
  duk_pop(ctx);

  duk_get_prop_string(ctx, -1, "\xff""\xff""uv_tcp");
  sock_id = duk_to_int(ctx, -1);
  duk_pop(ctx);

  duk_pop(ctx);

  printf("%s %d - %d\n", __PRETTY_FUNCTION__, __LINE__, sock_id);

  return 0;
}

static duk_ret_t net_socketOn(duk_context *ctx) {
  uv_tcp_t *tcp = NULL;
  int sock_id = 0;

  duk_push_this(ctx);

  duk_get_prop_string(ctx, -1, "\xff""\xff""handler");
  tcp = (uv_tcp_t *) duk_to_pointer(ctx, -1);
  duk_pop(ctx);

  duk_get_prop_string(ctx, -1, "\xff""\xff""uv_tcp");
  sock_id = duk_to_int(ctx, -1);
  duk_pop(ctx);

  duk_pop(ctx);

  const char *buf = duk_require_string(ctx, 0);
  if (strcmp(buf, "data") != 0 && strcmp(buf, "lookup") != 0 &&
    strcmp(buf, "connect") != 0 && strcmp(buf, "end") != 0 &&
    strcmp(buf, "timeout") != 0 && strcmp(buf, "drain") != 0 &&
    strcmp(buf, "error") != 0 && strcmp(buf, "close") != 0) {

    return 0;
  }

  if (!duk_is_function(ctx, 1))
    return 0;

  duk_push_global_stash(ctx); // [ stash ]
  duk_get_prop_string(ctx, -1, "sockHandler"); // [ stash sockHandler ]
  duk_push_int(ctx, sock_id); // [ stash sockHandler sock_id ]
  duk_get_prop(ctx, -2); // [ stash sockHandler obj ]

  duk_bool_t is_undefined = duk_is_undefined(ctx, -1);

  duk_push_global_stash(ctx);
  duk_get_prop_string(ctx, -1, "sockHandler");
  duk_push_int(ctx, sock_id); // [ stash sockHandler obj sock_id ]

  if (is_undefined) {
    duk_push_object(ctx); // [ stash sockHandler obj sock_id obj]
  } else {
    duk_dup(ctx, -4); // [ stash sockHandler obj sock_id obj]
    duk_bool_t is_obj = duk_is_object(ctx, -1);
  }

  duk_push_string(ctx, buf); // [ stash sockHandler obj sock_id obj action ]
  duk_dup(ctx, 1); // [ stash sockHandler obj sock_id obj action cb ]
  duk_put_prop(ctx, -3); // object[action] = callback, [ stash sockHandler obj sock_id obj ]
  duk_put_prop(ctx, -3); // sockHandler[id] = object, [ stash sockHandler obj ]
  duk_pop_n(ctx, 5);

  return 0;
}

const duk_function_list_entry net_socket_functions_list[] = {
  { "write", net_socketWrite, 3  },
  { "on", net_socketOn, 2  },
  { NULL, NULL, 0 }
};

static void net_on_new_connection(uv_stream_t *server, int status) {
  struct sockaddr_storage peer;
  struct sockaddr_in *in4;
  char ipaddr[32];
  int namelen, r;
  if (status == -1)
    return;

  uv_tcp_t *client = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), client);
  if (uv_accept(server, (uv_stream_t *) client) == 0) {
    printf("Accept Connection Ok\n");

    uv_read_start((uv_stream_t *) client, libuv_alloc_buffer, libuv_read_cb);

    duk_int_t result;
    int32_t id = (int32_t) server->data;
    int32_t sock_id = (int32_t) sock_next_id++;

    client->data = (void *) sock_id;

    duk_push_global_stash(mainContext);
    duk_get_prop_string(mainContext, -1, "netHandler");
    duk_push_number(mainContext, id);
    duk_get_prop(mainContext, -2);

    duk_push_object(mainContext);
    duk_push_pointer(mainContext, (void *) client);
    duk_put_prop_string(mainContext, -2, "\xff""\xff""handler");

    duk_push_number(mainContext, sock_id);
    duk_put_prop_string(mainContext, -2, "\xff""\xff""uv_tcp");

    namelen = sizeof(struct sockaddr_storage);
    r = uv_tcp_getpeername((uv_tcp_t *) client, (struct sockaddr *) &peer, &namelen);
    in4 = (struct sockaddr_in *) &peer;
    uv_inet_ntop(AF_INET, &in4->sin_addr, ipaddr, 32);

    duk_push_int(mainContext, htons(in4->sin_port));
    duk_put_prop_string(mainContext, -2, "remotePort");
    duk_push_string(mainContext, ipaddr);
    duk_put_prop_string(mainContext, -2, "remoteAddress");
    duk_push_string(mainContext, "IPv4");
    duk_put_prop_string(mainContext, -2, "remoteFamily");

    namelen = sizeof(struct sockaddr_storage);
    r = uv_tcp_getsockname((uv_tcp_t *) client, (struct sockaddr *) &peer, &namelen);
    in4 = (struct sockaddr_in *) &peer;
    uv_inet_ntop(AF_INET, &in4->sin_addr, ipaddr, 32);

    duk_push_int(mainContext, htons(in4->sin_port));
    duk_put_prop_string(mainContext, -2, "localPort");
    duk_push_string(mainContext, ipaddr);
    duk_put_prop_string(mainContext, -2, "localAddress");

    duk_put_function_list(mainContext, -1, net_socket_functions_list);
    result = duk_pcall(mainContext, 1);

    duk_pop(mainContext); // Result
    duk_pop(mainContext); // Net Handler
    duk_pop(mainContext); // Stash
  } else {
    free(client);
    uv_close((uv_handle_t *) client, NULL);
  }
}

static duk_ret_t net_createConnection(duk_context *ctx) {
  return 0;
}

static duk_ret_t net_serverListen(duk_context *ctx) {
  uv_tcp_t *tcp;
  struct sockaddr_in bind_addr;

  const char *address = duk_require_string(ctx, 0);
  duk_int_t port = duk_require_int(ctx, 1);

  duk_push_this(ctx);
  duk_get_prop_string(ctx, -1, "\xff""\xff""handler");
  tcp = (uv_tcp_t *) duk_to_pointer(ctx, -1);
  duk_pop(ctx);

  bind_addr = uv_ip4_addr(address, port);
  uv_tcp_bind(tcp, bind_addr);
  uv_listen((uv_stream_t *) tcp, 128, net_on_new_connection);

  return 0;
}

static duk_ret_t net_serverClose(duk_context *ctx) {
//  printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
  return 0;
}

static duk_ret_t net_serverAddress(duk_context *ctx) {
//  printf("%s %d\n", __PRETTY_FUNCTION__, __LINE__);
  return 0;
}

static duk_ret_t net_serverFinalizer(duk_context *ctx) {
  int32_t id;
  uv_tcp_t *ptr = 0;

  duk_get_prop_string(ctx, -1, "\xff""\xff""handler");
  ptr = duk_to_pointer(ctx, -1);
  duk_pop(ctx);

  printf("Final Ptr : %p\n", ptr);
  uv_close((uv_handle_t *) ptr, libuv_close);

  duk_get_prop_string(ctx, -1, "\xff""\xff""uv_tcp");
  id = (int32_t) duk_to_number(ctx, -1);
  duk_pop(ctx);

  duk_push_global_stash(ctx);
  duk_get_prop_string(ctx, -1, "netHandler");
  duk_push_number(ctx, id);
  duk_del_prop(ctx, -2);

  return 0;
}

const duk_function_list_entry net_server_functions_list[] = {
  { "listen", net_serverListen, 4  },
  { "close", net_serverClose, 4  },
  { "address", net_serverAddress, 0  },
  { NULL, NULL, 0 }
};

static void createServer(duk_context *ctx, int withOptions) {
  int32_t id = net_next_id++;

  duk_push_object(ctx); // Create Object
  duk_put_function_list(ctx, -1, net_server_functions_list); // Place Socket Functions List
  duk_push_c_function(ctx, net_serverFinalizer, 1); // Place the Finalizer
  duk_set_finalizer(ctx, -2);

  uv_tcp_t *tcp = (uv_tcp_t *) malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), tcp);
  tcp->data = (void *) id;
  duk_push_pointer(ctx, (void *) tcp);
  duk_put_prop_string(ctx, -2, "\xff""\xff""handler");

  duk_push_number(ctx, id);
  duk_put_prop_string(ctx, -2, "\xff""\xff""uv_tcp");

  duk_push_global_stash(ctx);
  duk_get_prop_string(ctx, -1, "netHandler");
  duk_push_number(ctx, id);
  duk_dup(ctx, withOptions != 0 ? 1 : 0);
  duk_put_prop(ctx, -3);
  duk_pop(ctx);
  duk_pop(ctx);
}

static duk_ret_t net_createServer(duk_context *ctx) {
  duk_bool_t rc = 0;
  int32_t timer_id = net_next_id++;
  duk_int_t p0 = duk_get_type(ctx, 0);
  duk_int_t p1 = duk_get_type(ctx, 1);

  if (duk_is_object(ctx, 0) && duk_is_callable(ctx, 0)) { // Function
    createServer(ctx, 0);
    return 1;
  } else if (duk_is_object(ctx, 0) && !duk_is_callable(ctx, 0) && duk_is_object(ctx, 1) && duk_is_callable(ctx, 1)) { // Object , Function
    createServer(ctx, 1);
    return 1;
  } else {
    return DUK_RET_ERROR;
  }

  return 0;
}

static duk_ret_t net_connect(duk_context *ctx) {
//  printf("%s %d - Top : %d\n", __PRETTY_FUNCTION__, __LINE__, duk_get_top(ctx));
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

  duk_push_global_stash(ctx);
  duk_push_object(ctx);
  duk_put_prop_string(ctx, -2, "netHandler");
  duk_push_object(ctx);
  duk_put_prop_string(ctx, -2, "sockHandler");
  duk_pop(ctx);
}

int main(int argc, char **argv) {
  uv_loop_t *loop = uv_default_loop();
  mainContext = duk_create_heap_default();

  net_Init(mainContext);
  duk_eval_file(mainContext, "tests/net_test.js");

  uv_run(loop, UV_RUN_DEFAULT);
  duk_destroy_heap(mainContext);

  return 0;
}
