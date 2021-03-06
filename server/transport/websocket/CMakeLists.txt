if(MSYS)
  # On MSYS the develop branch is required to compile against
  # newer Boost versions. Unfortunately that does not signal
  # any newer package version than 0.8.0, which is essentially
  # the same as the master branch.
  find_package(websocketpp REQUIRED)
else()
 find_package(websocketpp 0.7.0 REQUIRED)
endif()

set (WEBSOCKET_SOURCES
  WebSocketTransport.cpp
  WebSocketTransport.hpp
  WebSocketTransportFactory.cpp
  WebSocketTransportFactory.hpp
  WebSocketEventHandler.cpp
  WebSocketEventHandler.hpp
  WebSocketRegistrar.cpp
  WebSocketRegistrar.hpp
)

add_library(websocketTransport
  ${WEBSOCKET_SOURCES}
)
if(SANITIZERS_ENABLED)
  add_sanitizers(websocketTransport)
endif()

target_link_libraries(websocketTransport
  ${GSTREAMER_LIBRARIES}
  ${JSONRPC_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${KMSCORE_LIBRARIES}
)

set_property (TARGET websocketTransport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${JSONRPC_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${KMSCORE_INCLUDE_DIRS}
)
