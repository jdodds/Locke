/**
 * \file locke.c
 *
 *  Created on: 10/11/2011
 *      Author: Marcelo Elias Del Valle
 *
 *  \brief Application Server main file.
 *  This file includes the main function and corresponds to the app server itself.
 *
 */
#include <locke.h>
#include <locke_system.h>
#include <locke_appmanager.h>
#include <locke_application.h>

#include <stdio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib-object.h>
#include <signal.h>
#include <unistd.h>

gboolean timeout_callback(gpointer data) {
	static int i = 0;

	i++;
	g_print("timeout_callback called %d/10 times\n", i);
	if (10 == i) {
		g_main_loop_quit((GMainLoop*) data);
		return FALSE;
	}

	return TRUE;
}

static void signals_handler(int signum) {
	/* Any received signal will terminate the server */
	printf("PID (%d) Received signal [%d]!", getpid(), signum);
	switch (signum) {
	case SIGSEGV:
		/* segmentation fault - finishes server
		 * overrides signal handler to default and kill the process again, to generate the core file */
		signal(signum, SIG_DFL);
		kill(getpid(), signum);
		break;
	default:
		printf("Finishing server\n");
		locke_system_quit_mainloop(locke_system_get_singleton(0, NULL));
		break;
	}
	return;
}

int main(int argc, char *argv[]) {
	/* Setup program signals */
	signal(SIGINT, signals_handler);
	signal(SIGHUP, signals_handler);
	signal(SIGTERM, signals_handler);
	signal(SIGSEGV, signals_handler);

	g_print(
			"============================================================================\n");
	g_print("Locke server v%d.%d\n", LOCKE_MAJOR_VERSION, LOCKE_MINOR_VERSION);
	g_print(
			"============================================================================\n");
	g_print("Starting glib backend\n");
	/* Initialize GLib type system */
	g_type_init();

	g_print("Creating main loop\n");
	/* Init system */
	LockeSystem *system = locke_system_get_singleton(argc, argv);
	locke_system_set_serverpid(system, getpid());

	/* Create application manager */
	gchar deployFolder[1024];

	strcpy(deployFolder, system->appFolder);
	strcat(deployFolder, "autodeploy");
	LockeAppManager *appmanager = locke_appmanager_new();

	GError *err = NULL;
	locke_appmanager_set_state(appmanager, SERVER_STARTING);
	locke_appmanager_init(appmanager, deployFolder, &err);
	if (err != NULL) {
		/* Report error to user, and free error */
		fprintf(stderr, "Unable to Init application manager: %s\n",
				err->message);
		g_error_free(err);
		goto locke_main_finally;
	}
	/* Applications may have been started on init call
	 * If so, do not start server loop */
	if (locke_system_get_child(system) == FALSE) {
		locke_appmanager_set_state(appmanager, SERVER_RUNNING);
		/* Run the main loop */
		locke_system_start_mainloop(system);
	}
	/*
	 * Applications may have been started after main loop was running. If so,
	 * destroy the unused memory and start the app
	 * */
	if (locke_system_get_child(system) == TRUE) { /* It's the application child process created */
		/*child process won't need server memory */
		/* locke_appmanager_destroy(appmanager); */
		LockeApplication *app = locke_application_get_singleton();
		locke_application_run(app);
		locke_application_destroy_singleton();
		return 0;
	}
	/* main loop done*/
	locke_appmanager_set_state(appmanager, SERVER_STOPPING);

	locke_appmanager_stop(appmanager);
	locke_main_finally: locke_appmanager_set_state(appmanager, SERVER_STOPPED);
	locke_appmanager_destroy(appmanager);
	return 0;
}

