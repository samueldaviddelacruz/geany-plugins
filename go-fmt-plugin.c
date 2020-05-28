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
	
	
	int is_go_file_type = strcmp(doc->file_type->extension, GO_FILE_EXTENSION) == false;
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


		//create temp file to hold the formatted text for gofmt to read from
		char tempFileNameBuff[32] = "/tmp/myTmpFile-XXXXXX";
		int tempFileDescriptor = mkstemp(tempFileNameBuff);

		 if(tempFileDescriptor<1)
		{
			printf("\n Creation of temp file failed with error [%s]\n","");
			return;
		}
		
		char *doc_content = sci_get_contents(sci,sci_len);
		close(tempFileDescriptor);

		//open file and write current document content to it
		FILE *tempFilePointer;
		tempFilePointer = fopen(tempFileNameBuff, "w+");
		fputs(doc_content, tempFilePointer);
		fclose(tempFilePointer);

		//create command string to gofmt the temp file
		char cmd_buffer[256];
		//"%s %s 2>&1",
		sprintf(cmd_buffer, "%s %s", "gofmt", tempFileNameBuff );
		FILE *fp;

		char path[4096];
	
		GError **error;
		GString *sdoutput = g_string_new(NULL);
		GString *stderroutput = g_string_new(NULL);
		bool success = spawn_sync(NULL,cmd_buffer,NULL,NULL,NULL,sdoutput,stderroutput,NULL,error);
		
		if(success == true){
			if(stderroutput->len > 0){		
				gchar **errors = g_strsplit(stderroutput->str,"\n",-1);
				int i = 0;
				while(errors[i] != NULL) {
					GString *error = g_string_new(errors[i]);
					utils_string_replace_all(error, tempFileNameBuff, "Error at line"); 
					msgwin_compiler_add(COLOR_RED,"%s",error->str);
					i++;
				}				
				return;
			}else{
				g_string_append(formatted, sdoutput->str);
			}					
		}
		
		
		changed = (formatted->len != sci_len) ||
              (g_strcmp0(formatted->str, sci_buf) != 0);
		
		old_first_line = scintilla_send_message(sci, SCI_GETFIRSTVISIBLELINE, 0, 0);
		

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

		
		// remove temp files temporary file is deleted
		remove(tempFileNameBuff);
		
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
