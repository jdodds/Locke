bin_PROGRAMS = locke
locke_SOURCES = locke.c locke_appmanager.c locke_application.c locke_system.c locke_application_wrapper.c locke_service_manager.c locke_service.c locke_service_socket.c locke_service_http.c
locke_CFLAGS = -Wall --ansi -D__USE_POSIX -I. -I/usr/include -I/usr/include/i386-linux-gnu/ -I../api/ $(GLIB_CFLAGS) $(GIO_CFLAGS) $(GMODULE_CFLAGS)
locke_LDFLAGS = $(GLIB_LIBS) $(GIO_LIBS) $(GMODULE_LIBS)
