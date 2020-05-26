/* License blob */
#include <geanyplugin.h>
static gboolean hello_init(GeanyPlugin *plugin, gpointer pdata)
{
    printf("Hello World from plugin!\n");
    /* Perform advanced set up here */
    return TRUE;
}
static void hello_cleanup(GeanyPlugin *plugin, gpointer pdata)
{
    printf("Bye World :-(\n");
}
G_MODULE_EXPORT
void geany_load_module(GeanyPlugin *plugin)
{
    /* Step 1: Set metadata */
    plugin->info->name = "HelloWorld";
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
