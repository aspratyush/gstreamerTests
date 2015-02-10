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

	int gstStatus = 0;
	//define the elements
	GstElement *pipeline, *filesrc, *oggdemux, *decoder, *converter, *audioOutput;
	GstBus *msgBus;
	guint bus_watch_id;
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, FALSE);

	/* Check input arguments */
	if (argc != 2) {
		g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
		return -1;
	}

	//--------------1) VERSION CHECK--------------//
	gstVersionCheck(argc, argv);

	//--------------2) Ogg-player------------------//
	g_print("gstOggPlayer test...\n");

	//--------3) initialize the elements----------//
	gstStatus = gstInitializeElements(pipeline, filesrc, oggdemux, decoder,
			converter, audioOutput, msgBus);
	if (gstStatus){
		return -1;
	}
	g_print("Init done.. assigning file\n");

	/* we set the input filename to the source element */
	g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);

	//-------4) Message Handler-------------------//
	bus_watch_id = gst_bus_add_watch(msgBus, busCallBack, loop);
	g_print("Added bus_watch\n");
	//unref the bus object
	gst_object_unref(msgBus);
	g_print("Unref mshBus object\n");

	//------------5) Add to bin and link the elements-----------//
	gst_bin_add_many( GST_BIN(pipeline), filesrc, oggdemux, decoder, converter,
																audioOutput, NULL);
	g_print("Add elements to bin\n");

	gst_element_link(filesrc, oggdemux);
	g_print("Link elements\n");

	gst_element_link_many(decoder, converter, audioOutput, NULL);
	g_print("Link many elements\n");

	//-----------Add listener for "sometimes" pad-add---------//
	g_signal_connect(oggdemux, "pad-added", G_CALLBACK(on_pad_added), decoder);
	g_print("Connect signal\n");

	/* Set the pipeline to "playing" state*/
	g_print ("Now playing: %s\n", argv[1]);
	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	/* Iterate */
	g_print ("Running...\n");
	g_main_loop_run (loop);

	/* Out of the main loop, clean up nicely */
	g_print ("Returned, stopping playback\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);

	g_print ("Deleting pipeline\n");
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
void gstVersionCheck(int argc, char *argv[]){
	gchar *str;
	guint major, minor, micro, nano;

	//init gst
	gst_init(&argc, &argv);

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
 *    This function initializes the elements of the pipeline
 *
 *  @return
 *    int
 *
 *  @param[in]
 *    GstElement(s)
 *
 *  @param[out]
 *    GstElements(s)
 *
 ******************************************************************************
 */
int gstInitializeElements(GstElement *pipeline, GstElement *filesrc,
		GstElement *oggdemux, GstElement *decoder, GstElement *converter,
		GstElement *audioOutput, GstBus *msgBus){

	//initialize elements
	pipeline = gst_pipeline_new("oggAudioPlayer");
	filesrc = gst_element_factory_make("filesrc", "file-src");
	oggdemux = gst_element_factory_make("oggdemux", "ogg-demux");
	decoder = gst_element_factory_make("vorbisdec", "vorbis-decoder");
	converter = gst_element_factory_make("audioconvert", "converter");
	audioOutput = gst_element_factory_make("autoaudiosink", "audio-output");

	//initialize message bus
	msgBus = gst_pipeline_get_bus( GST_PIPELINE(pipeline) );

	//check if any element has not been initialized
	if (!pipeline | !filesrc | !oggdemux | !decoder | !converter | !audioOutput
			| !msgBus ){
		g_print("One of the pipeline elements failed to initialize\n");
		return -1;
	}

	g_print("Pipeline elements initialized!\n");
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
int busCallBack( GstBus *bus, GstMessage *message, gpointer data ){
	GMainLoop *loop = (GMainLoop *) data;

	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_EOS:{
			g_print ("End of stream\n");
			g_main_loop_quit (loop);
			break;
		}
		case GST_MESSAGE_ERROR: {
			gchar *debug;
			GError *error;
			gst_message_parse_error (message, &error, &debug);
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
static void on_pad_added(GstElement *element, GstPad *pad, gpointer data){
	//create sinkpad
	GstPad *sinkpad;
	// cast data as GstElement (for decoder element)
	GstElement *decoder = (GstElement *) data;

	/* We can now link this pad with the vorbis-decoder sink pad */
	g_print ("Dynamic pad created, linking demuxer/decoder\n");
	sinkpad = gst_element_get_static_pad (decoder, "sink");

	//link current pad to sinkpad
	gst_pad_link (pad, sinkpad);
	gst_object_unref (sinkpad);
}
