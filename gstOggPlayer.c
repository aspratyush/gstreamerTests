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
	GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
	GstBus *bus;
	guint bus_watch_id;
	/* Initialisation */
	gst_init (&argc, &argv);
	loop = g_main_loop_new (NULL, FALSE);
	/* Check input arguments */
	if (argc != 2) {
	g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
	return -1;
	}
	/* Create gstreamer elements */
	pipeline = gst_pipeline_new ("audio-player");
	source	= gst_element_factory_make ("filesrc","file-source");
	demuxer = gst_element_factory_make ("oggdemux","ogg-demuxer");
	decoder = gst_element_factory_make ("vorbisdec","vorbis-decoder");
	conv	= gst_element_factory_make ("audioconvert","converter");
	sink	= gst_element_factory_make ("autoaudiosink","audio-output");

	if (!pipeline || !source || !demuxer || !decoder || !conv || !sink) {
	g_printerr ("One element could not be created. Exiting.\n");
	return -1;
	}
	/* Set up the pipeline */
	/* we set the input filename to the source element */
	g_object_set (G_OBJECT (source), "location", argv[1], NULL);
	/* we add a message handler */
	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	bus_watch_id = gst_bus_add_watch (bus, busCallBack, loop);
	gst_object_unref (bus);
	/* we add all elements into the pipeline */
	/* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
	gst_bin_add_many (GST_BIN (pipeline),
	source, demuxer, decoder, conv, sink, NULL);
	/* we link the elements together */
	/* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
	gst_element_link (source, demuxer);
	gst_element_link_many (decoder, conv, sink, NULL);
	g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), decoder);
	/* note that the demuxer will be linked to the decoder dynamically.
	The reason is that Ogg may contain various streams (for example
	audio and video). The source pad(s) will be created at run time,
	by the demuxer when it detects the amount and nature of streams.
	Therefore we connect a callback function which will be executed
	when the "pad-added" is emitted.*/
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
//void gstVersionCheck(int argc, char *argv[]){
//	gchar *str;
//	guint major, minor, micro, nano;
//
//	//init gst
//	gst_init(&argc, &argv);
//
//	//version check
//	gst_version(&major, &minor, &micro, &nano);
//	if (nano == 1){
//		str = "(CVS)";
//	}
//	else if(nano == 2){
//		str = "(Prerelease)";
//	}
//	else{
//		str = "None";
//	}
//	g_print("Gstreamer version : %d.%d.%d %s\n",major,minor,micro,str);
//	g_print("-----------------------------------\n");
//
//}

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
//int gstInitializeElements(GstElement *pipeline, GstElement *filesrc,
//		GstElement *oggdemux, GstElement *decoder, GstElement *converter,
//		GstElement *audioOutput, GstBus *msgBus){
//
//	//initialize elements
//	pipeline = gst_pipeline_new("oggAudioPlayer");
//	filesrc = gst_element_factory_make("filesrc", "file-src");
//	oggdemux = gst_element_factory_make("oggdemux", "ogg-demux");
//	decoder = gst_element_factory_make("vorbisdec", "vorbis-decoder");
//	converter = gst_element_factory_make("audioconvert", "converter");
//	audioOutput = gst_element_factory_make("autoaudiosink", "audio-output");
//
//	//initialize message bus
//	msgBus = gst_pipeline_get_bus( GST_PIPELINE(pipeline) );
//
//	//check if any element has not been initialized
//	if (!pipeline | !filesrc | !oggdemux | !decoder | !converter | !audioOutput
//			| !msgBus ){
//		g_print("One of the pipeline elements failed to initialize\n");
//		return -1;
//	}
//
//	g_print("Pipeline elements initialized!\n");
//	return 0;
//}

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
