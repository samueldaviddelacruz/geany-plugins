/* License blob */
#include <geanyplugin.h>
#include <document.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Scintilla.h>
#include <ScintillaWidget.h>

//Special thanks to codebrainz(https://github.com/codebrainz) for creating the code-format plugin for C/C++
//specially this part => https://github.com/codebrainz/code-format/blob/b13cdb372fdaa26cf67ab745317727e2881936eb/plugin.c#L315
const char GO_FILE_EXTENSION[10] = "go";
static void document_before_save(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
	printf("Example: %s is about to be saved\n", DOC_FILENAME(doc));

	int is_go_file_type = strcmp(doc->file_type->extension, GO_FILE_EXTENSION) == 0;
	if (doc == NULL)
		doc = document_get_current();

	if (!DOC_VALID(doc))
	{
		g_warning("Cannot format with no documents open");
		return;
	}
	if (is_go_file_type)
	{

		GString *formatted;
		ScintillaObject *sci;
		size_t offset = 0, length = 0, sci_len;
		size_t cursor_pos, old_first_line, new_first_line, line_delta;
		const char *sci_buf;
		bool changed = true;
		bool was_changed;

		was_changed = doc->changed;
		sci = doc->editor->sci;

		//format entire doc on save
		offset = 0;
		length = sci_get_length(sci);

		cursor_pos = sci_get_current_position(sci);
		sci_len = sci_get_length(sci);

		sci_buf = (const char *)scintilla_send_message(sci, SCI_GETCHARACTERPOINTER, 0, 0);

		formatted = g_string_sized_new(sci_len);

		char cmd_buffer[256];

		sprintf(cmd_buffer, "%s %s", "gofmt", DOC_FILENAME(doc));
		FILE *fp;
		char path[4096];

		/* Open the command for reading. */
		fp = popen(cmd_buffer, "r");
		if (fp == NULL)
		{
			g_warning("Failed to run command");
			return;		
		}

		/* Read the output a line at a time - output it. */
		while (fgets(path, sizeof(path), fp) != NULL)
		{
			g_string_append(formatted, path);
			printf("%s", path);
		}

		old_first_line = scintilla_send_message(sci, SCI_GETFIRSTVISIBLELINE, 0, 0);
		printf("%s", formatted->str);

		// Replace document text and move cursor to new position
		scintilla_send_message(sci, SCI_BEGINUNDOACTION, 0, 0);
		scintilla_send_message(sci, SCI_CLEARALL, 0, 0);
		scintilla_send_message(sci, SCI_ADDTEXT, formatted->len,
							   (sptr_t)formatted->str);
		scintilla_send_message(sci, SCI_GOTOPOS, cursor_pos, 0);
		new_first_line = scintilla_send_message(sci, SCI_GETFIRSTVISIBLELINE, 0, 0);
		line_delta = new_first_line - old_first_line;
		scintilla_send_message(sci, SCI_LINESCROLL, 0, -line_delta);
		scintilla_send_message(sci, SCI_ENDUNDOACTION, 0, 0);

		document_set_text_changed(doc, (was_changed || changed));

		/* close */
		g_string_free(formatted, true);
		pclose(fp);
	}
}
static gboolean hello_init(GeanyPlugin *plugin, gpointer pdata)
{


	plugin_signal_connect(plugin, NULL, "document-before-save", TRUE,
						  G_CALLBACK(document_before_save), plugin);
	return TRUE;
}

static void hello_cleanup(GeanyPlugin *plugin, gpointer pdata)
{
}

G_MODULE_EXPORT
void geany_load_module(GeanyPlugin *plugin)
{
	/* Step 1: Set metadata */
	plugin->info->name = "Go Format";
	plugin->info->description = "Geany Plugin for gofmt on save";
	plugin->info->version = "1.0";
	plugin->info->author = "Samuel De La Cruz <alphaelena@gmail.com>";
	/* Step 2: Set functions */
	plugin->funcs->init = hello_init;
	plugin->funcs->cleanup = hello_cleanup;

	/* Step 3: Register! */
	GEANY_PLUGIN_REGISTER(plugin, 225);
	/* alternatively:
    GEANY_PLUGIN_REGISTER_FULL(plugin, 225, data, free_func); */
}
