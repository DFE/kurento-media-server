set (TRANSPORT_SOURCES
  Processor.hpp
  Transport.hpp
  TransportFactory.cpp
  TransportFactory.hpp
)

add_library (transport ${TRANSPORT_SOURCES})
if(SANITIZERS_ENABLED)
  add_sanitizers(transport)
endif()

add_dependencies(transport websocketTransport)

if(MSYS)
set(TRANSPORT_LINK_OS_LIBRARIES ws2_32 mswsock)
endif()

target_link_libraries(transport
  ${GSTREAMER_LIBRARIES}
  ${KMSCORE_LIBRARIES}
  websocketTransport
  ${TRANSPORT_LINK_OS_LIBRARIES}
)

set_property (TARGET transport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/websocket/
    ${GSTREAMER_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${KMSCORE_INCLUDE_DIRS}
)

add_subdirectory(websocket)
