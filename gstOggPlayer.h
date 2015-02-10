/*
 * gstOggPlayer.h
 *
 *  Created on: 10-Feb-2015
 *      Author: pratyush
 */
#ifndef GSTOGGPLAYER_H_
#define GSTOGGPLAYER_H_

#include <gst/gst.h>

//check gstreamer version
void gstVersionCheck();

//create bus-callback
int busCallBack( GstBus *bus, GstMessage *message, gpointer data );

//handle pad-addition for sometimes pad
static void on_pad_added(GstElement *element, GstPad *pad, gpointer data);

#endif /* GSTOGGPLAYER_H_ */
