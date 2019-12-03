#pragma once

enum ErrorCode_t
{
   STATUS_SUCCESS,
   ERR_TRANSPORT_SOCKET_CREATION_FAILED,
   ERR_TRANSPORT_BIND_FAILED,
   ERR_TRANSPORT_LISTEN_FAILED,
   ERR_TRANSPORT_FAILED_TO_LOCATE_SERVER_PORT,
   ERR_TRANSPORT_FAIL_TO_XMIT_ALL_DATA,
   ERR_TRANSPORT_CONNECTION_CLOSED,
   ERR_TRANSPORT_CONNECTION_FAILED_TO_CREATE_SOCKET,
   ERR_TRANSPORT_CONNECTION_FAIL_TO_ESTABLISH_CONNECTION,
   ERR_TRANSPORT_CONNECTION_FAIL_TO_PARSER_HOST_NAME,
   ERR_TRANSPORT_CONNECTION_FAULT,
   ERR_CLUSTER_INIT_IN_POGRESS,
};
