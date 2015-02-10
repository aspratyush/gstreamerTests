//============================================================================
// Name        : gstOggPlayer.c
// Author      : PSahay
// Version     :
// Copyright   : GPLv3
// Description : Hello World in C, Ansi-style
//============================================================================

#include "gstOggPlayer.h"
#include <glib.h>

int main(int argc, char* argv[]){

	GMainLoop *loop;
	GstElement *pipeline, *filesrc, *oggdemux, *decoder, *converter, *audioOutput;
	GstBus *msgBus;
	guint bus_watch_id;
	/* Initialisation */
	gst_init (&argc, &argv);
	loop = g_main_loop_new (NULL, FALSE);
	/* Check input arguments */
	if (argc != 2) {
		g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
		return -1;
	}

	//--------------1) Init & Version Check--------------//
	gst_init(&argc, &argv);
	gstVersionCheck();

	//--------------2) Ogg-player------------------//
	g_print("gstOggPlayer test...\n");

	/* Check input arguments */
	if (argc != 2) {
		g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
		return -1;
	}

	//--------3) initialize the elements----------//
	int status = gstInitElements(&pipeline, &filesrc, &oggdemux, &decoder, &converter,
													&audioOutput, &msgBus);
	if (status){
		return -1;
	}
	g_print("Pipeline elements initialized!\n");

	//-------4) Message Handler-------------------//
	bus_watch_id = gst_bus_add_watch(msgBus, busCallBack, loop);
	// unref the bus object
	gst_object_unref(msgBus);

	//	we set the input filename to the source element
	g_print("Init done.. assigning file\n");
	g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);

	//------------5) Add to bin and link the elements-----------//
	gst_bin_add_many( GST_BIN(pipeline), filesrc, oggdemux, decoder, converter,
																audioOutput, NULL);
	gst_element_link(filesrc, oggdemux);
	gst_element_link_many(decoder, converter, audioOutput, NULL);

	//-----------6) Add listener for "sometimes" pad-add---------//
	g_signal_connect(oggdemux, "pad-added", G_CALLBACK(on_pad_added), decoder);

	// Set the pipeline to "playing" state
	g_print ("Now playing: %s\n", argv[1]);
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
	// Iterate
	g_main_loop_run (loop);
	// Out of the main loop, clean up nicely
	g_print ("Returned, stopping playback\n");
	g_print ("Deleting pipeline\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (pipeline));
	g_source_remove (bus_watch_id);
	g_main_loop_unref (loop);
	return 0;
}

/*!
 ******************************************************************************
 *  @brief
 *    This function queries the gstreamer version
 *
 *  @return
 *    void
 *
 *  @param[in]
 *    int argc, char *argv[]
 *
 *  @param[out]
 *    None
 *
 ******************************************************************************
 */
void gstVersionCheck(){
	gchar *str;
	guint major, minor, micro, nano;

	//version check
	gst_version(&major, &minor, &micro, &nano);
	if (nano == 1){
		str = "(CVS)";
	}
	else if(nano == 2){
		str = "(Prerelease)";
	}
	else{
		str = "None";
	}
	g_print("Gstreamer version : %d.%d.%d %s\n",major,minor,micro,str);
	g_print("-----------------------------------\n");

}

/*!
 ******************************************************************************
 *  @brief
 *    This function queries the gstreamer version
 *
 *  @return
 *    void
 *
 *  @param[in]
 *    int argc, char *argv[]
 *
 *  @param[out]
 *    None
 *
 ******************************************************************************
 */
int gstInitElements(GstElement **pipeline, GstElement **filesrc,
		GstElement **oggdemux, GstElement **decoder, GstElement **converter,
		GstElement **audioOutput, GstBus **msgBus){
	//--------3) initialize the elements----------//
	*pipeline = gst_pipeline_new("oggAudioPlayer");
	*filesrc = gst_element_factory_make("filesrc", "file-src");
	*oggdemux = gst_element_factory_make("oggdemux", "ogg-demux");
	*decoder = gst_element_factory_make("vorbisdec", "vorbis-decoder");
	*converter = gst_element_factory_make("audioconvert", "converter");
	*audioOutput = gst_element_factory_make("autoaudiosink", "audio-output");

	//initialize message bus
	*msgBus = gst_pipeline_get_bus( GST_PIPELINE(*pipeline) );

	//check if any element has not been initialized
	if (!*pipeline | !*filesrc | !*oggdemux | !*decoder | !*converter | !*audioOutput
			| !*msgBus ){
		g_print("One of the pipeline elements failed to initialize\n");
		return -1;
	}

	return 0;
}

/*!
 ******************************************************************************
 *  @brief
 *    This function is the callback for pipeline messages
 *
 *  @return
 *    int
 *
 *  @param[in]
 *    GstBus, GstMessage, gpointer
 *
 *  @param[out]
 *    GstElements(s)
 *
 ******************************************************************************
 */
static gboolean busCallBack (GstBus *bus, GstMessage *msg, gpointer data){
		GMainLoop *loop = (GMainLoop *) data;
		switch (GST_MESSAGE_TYPE (msg)) {
			case GST_MESSAGE_EOS:{
				g_print ("End of stream\n");
				g_main_loop_quit (loop);
				break;
			}
			case GST_MESSAGE_ERROR: {
				gchar *debug;
				GError *error;
				gst_message_parse_error (msg, &error, &debug);
				g_free (debug);
				g_printerr ("Error: %s\n", error->message);
				g_error_free (error);
				g_main_loop_quit (loop);
				break;
			}
			default:
				break;
		}
	return TRUE;
	}

/*!
 ******************************************************************************
 *  @brief
 *    This function handles pad addition callback for sometimes pads
 *
 *  @return
 *    void
 *
 *  @param[in]
 *    GstElement, GstPad, gpointer
 *
 *  @param[out]
 *    None
 *
 ******************************************************************************
 */
static void on_pad_added (GstElement *element, GstPad *pad, gpointer data)
{
	GstPad *sinkpad;
	GstElement *decoder = (GstElement *) data;
	/* We can now link this pad with the vorbis-decoder sink pad */
	g_print ("Dynamic pad created, linking demuxer/decoder\n");
	sinkpad = gst_element_get_static_pad (decoder, "sink");
	gst_pad_link (pad, sinkpad);
	gst_object_unref (sinkpad);
}
