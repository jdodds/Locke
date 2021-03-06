/*
 * locke_application.c
 *
 *  Created on: 16/11/2011
 *      Author: mvalle
 */

#include <locke_application.h>
#include <locke_system.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <locke_log.h>

LockeApplication *locke_application_instance = NULL;
LockeApplication *locke_application_get_singleton() {
	if (locke_application_instance == NULL) {
		locke_application_instance = locke_application_new();
	}
	return locke_application_instance;
}

void locke_application_signal_handler(int signum) {
	/* Any received signal will terminate the server */
	g_log(LSVR_DOMAIN, G_LOG_LEVEL_CRITICAL, "PID (%d) Received signal [%d]!", getpid(), signum);
	switch (signum) {
	case SIGSEGV:
		/* segmentation fault - finishes server
		 * overrides signal handler to default and kill the process again, to generate the core file */
		signal(signum, SIG_DFL);
		kill(getpid(), signum);
		break;
	default:
		g_log(LSVR_DOMAIN, G_LOG_LEVEL_CRITICAL, "Finishing application ");
		locke_system_quit_mainloop(locke_system_get_singleton());
		break;
	}
	return;
}

GQuark locke_application_error_quark(void) {
	return g_quark_from_static_string("locke_application_error_quark");
}

LockeApplication *locke_application_new() {
	return (LockeApplication *) calloc(1, sizeof(LockeApplication));
}

/*
 * This is the 'main' function for the application process
 * the instance variables should have been already populated by the server
 * process
 *
 */
void locke_application_run(LockeApplication *app) {
	/* Setup program signals */
	signal(SIGINT, locke_application_signal_handler);
	signal(SIGHUP, locke_application_signal_handler);
	signal(SIGTERM, locke_application_signal_handler);
	signal(SIGSEGV, locke_application_signal_handler);

	g_log(LAPP_DOMAIN, G_LOG_LEVEL_DEBUG,
			"============================================================================");
	g_log(LAPP_DOMAIN, G_LOG_LEVEL_DEBUG, "PID(%d) locke_application_run called on the child process!!! ",
			getpid());
	g_log(LAPP_DOMAIN, G_LOG_LEVEL_DEBUG,
			"============================================================================");

	GError *err = NULL;
	locke_application_load(app, &err);
	if (err != NULL) {
		/* Report error to user, and free error */
		g_log(LSVR_DOMAIN, G_LOG_LEVEL_ERROR,
				"Unable to Init application. I won't init app main loop, bye! Details: '%s'",
				err->message);
		g_error_free(err);
		return;
	}

	app->events.lockeapp_on_start();

	locke_system_start_mainloop(locke_system_get_singleton());
}

void locke_application_load_callback(LockeApplication *app, const char* name,
		gpointer *method_pointer, GError **error) {
	if (!app->loaded_module) {
		g_set_error(error, LAPP_MODULE_ERROR, LAPP_MODULE_LOAD_ERROR,
				"Can't load callback '%s' from module, as it was not loaded!",
				name);
		return;
	}
	if (!g_module_symbol(app->loaded_module, name,
			(gpointer *) method_pointer)) {
		g_set_error(error, LAPP_MODULE_ERROR, LAPP_MODULE_NULL,
				"Can't load symbol %s from module '%s': Error: '%s'", name,
				g_file_get_path(app->appDllFile), g_module_error());
		return;
	}
	if (*method_pointer == NULL) {
		g_set_error(error, LAPP_MODULE_ERROR, LAPP_MODULE_OPEN_ERROR,
				"symbol %s is NULL", name);
		return;
	}
}

void locke_application_load(LockeApplication *app, GError **error) {
	app->loaded_module = g_module_open(g_file_get_path(app->appDllFile),
			G_MODULE_BIND_LAZY);
	if (!app->loaded_module) {
		g_set_error(error, LAPP_MODULE_ERROR, LAPP_MODULE_LOAD_ERROR, "%s",
				g_module_error());
		return;
	}
	locke_application_load_callback(app, "lockeapp_on_start",
			(gpointer *) &(app->events.lockeapp_on_start), error);
	if (*error != NULL)
		return;
	locke_application_load_callback(app, "lockeapp_on_stop",
			(gpointer *) &(app->events.lockeapp_on_stop), error);
	if (*error != NULL)
		return;
	locke_application_load_callback(app, "lockeapp_on_request",
			(gpointer *) &(app->events.lockeapp_on_request), error);
	if (*error != NULL)
		return;

	app->config = g_key_file_new();
	g_key_file_load_from_file(app->config, g_file_get_path(app->configFile),
			G_KEY_FILE_NONE, error);
	if (*error != NULL)
		return;
	gchar *service_type_str = g_key_file_get_value(app->config, "SERVICE",
			"type", error);
	if (service_type_str == NULL)
		return;
	gchar *port_str = g_key_file_get_value(app->config, "SERVICE", "port",
			error);
	if (port_str == NULL)
		return;
	LockeServiceManager *service_manager =
			locke_service_manager_get_singleton();
	app->service = locke_service_manager_lookup_service(service_manager,
			service_type_str);
	app->port = atoi(port_str);

	/* if configured service type can't be found, generate a new exception */
	if (app->service == NULL) {
		g_set_error(error, LAPP_MODULE_ERROR, LAPP_MODULE_LOAD_ERROR,
				"Couldn't find service type '%s'", service_type_str);
		return;
	}
	/*Check if port is valid */
	if (app->port <= 0) {
		g_set_error(error, LAPP_MODULE_ERROR, LAPP_MODULE_LOAD_ERROR,
				"Invalid port setting '%s'", port_str);
		return;
	}

	locke_service_listen_port(app->service, app, app->port, app->events.lockeapp_on_request	);
	g_log(LAPP_DOMAIN, G_LOG_LEVEL_INFO, "Servicing '%s' on port '%d'\n", service_type_str, app->port);

}

void locke_application_destroy_singleton() {
	if (locke_application_instance == NULL)
		return;
	locke_application_destroy(locke_application_instance);
	locke_application_instance = NULL;
}
void locke_application_destroy(LockeApplication *app) {
	g_log(LAPP_DOMAIN, G_LOG_LEVEL_DEBUG,
			"============================================================================\n");
	g_log(LAPP_DOMAIN, G_LOG_LEVEL_DEBUG, "PID(%d) Destroying application \n ", getpid());
	if (app->loaded_module != NULL) {
		app->events.lockeapp_on_stop();
		if (!g_module_close(app->loaded_module))
			g_warning("%s: %s", g_file_get_path(app->appDllFile),
					g_module_error());
	}
	g_object_unref(app->deployFolder);
	g_object_unref(app->appFolder);
	g_object_unref(app->appDllFile);
	g_object_unref(app->configFile);
	if (app->config != NULL)
		g_key_file_free(app->config);
	g_log(LAPP_DOMAIN, G_LOG_LEVEL_DEBUG,
			"============================================================================\n");
}

void locke_application_init(LockeApplication *app, const gchar *baseDir,
		const gchar *filename) {
	gchar fullpath[2048];
	gchar appfolder[2048];

	/* module (dll) */
	app->loaded_module = NULL;

	/* file name is the basename of the app */
	strcpy(app->basename, filename);

	app->deployFolder = g_file_new_for_path(baseDir);

	/* calculates app folder*/
	strcpy(appfolder, baseDir);
	if (appfolder[strlen(baseDir) - 1] != G_DIR_SEPARATOR)
		strcat(appfolder, G_DIR_SEPARATOR_S);
	strcat(appfolder, filename);
	strcat(appfolder, G_DIR_SEPARATOR_S);
	app->appFolder = g_file_new_for_path(appfolder);

	/* Calculates .so  path */
	strcpy(fullpath, appfolder);
	strcat(fullpath, "lib");
	strcat(fullpath, filename);
	strcat(fullpath, ".");
	strcat(fullpath, DLL_EXT);
	app->appDllFile = g_file_new_for_path(fullpath);

	/* Calculates .ini path */
	strcpy(fullpath, appfolder);
	strcat(fullpath, filename);
	strcat(fullpath, ".ini");
	app->configFile = g_file_new_for_path(fullpath);
}

