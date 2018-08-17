set( NullBackend_SRCS
  nullbackend.cpp
  nullsession.cpp
  nullexpression.cpp
  nullcompletionobject.cpp
)

add_backend(nullbackend ${NullBackend_SRCS})
